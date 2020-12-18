// Copyright (c) 2005 - 2018 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "FrameCounter.h"
#include "RTTR_Assert.h"
#include "helpers/win32_nanosleep.h" // IWYU pragma: keep
#include <algorithm>
#include <cmath>
#include <map>

//-V:clock::time_point:813

FrameCounter::FrameCounter(clock::duration updateInverval)
    : updateInverval_(updateInverval), framerate_(0), curNumFrames_(0)
{}

void FrameCounter::update(clock::time_point curTime)
{
    lastUpdateTime_ = curTime;
    if(curNumFrames_ == 0)
        curStartTime_ = curTime;
    else
    {
        if(lastUpdateTime_ - curStartTime_ >= updateInverval_)
        {
            framerate_ = getCurFrameRate();
            curStartTime_ = curTime;
            curNumFrames_ = 0;
        }
    }
    curNumFrames_++;
}

unsigned FrameCounter::getCurFrameRate() const
{
    clock::duration timeDiff = lastUpdateTime_ - curStartTime_;
    if(timeDiff == clock::duration::zero())
        return 0;
    using dSeconds = std::chrono::duration<double>;
    return std::lround(curNumFrames_ / std::chrono::duration_cast<dSeconds>(timeDiff).count());
}

FrameTimer::FrameTimer(int targetFramerate, unsigned maxLagFrames, clock::time_point curTime)
    : targetFrameDuration_(duration_t::zero()), maxLagFrames_(maxLagFrames), lastUpdateTime_(curTime),
      nextFrameTime_(curTime), totalLagShift_(duration_t::zero())
{
    setTargetFramerate(targetFramerate);
}

void FrameTimer::setTargetFramerate(int targetFramerate)
{
    using namespace std::chrono;
    duration_t targetDuration;
    if(targetFramerate <= 0)
        targetDuration = duration_t::zero(); // Disabled
    else
        targetDuration = duration_cast<duration_t>(seconds(1)) / std::min(targetFramerate, 200); // Max 200FPS
    setTargetFrameDuration(targetDuration);
}

void FrameTimer::setTargetFrameDuration(duration_t targetDuration)
{
    nextFrameTime_ -= targetFrameDuration_;

    targetFrameDuration_ = targetDuration;

    nextFrameTime_ += targetFrameDuration_;
}

FrameTimer::duration_t FrameTimer::calcTimeToNextFrame(clock::time_point curTime) const
{
    using namespace std::chrono;

    if(targetFrameDuration_ == duration_t::zero())
        return clock::duration::zero();

    if(curTime < nextFrameTime_)
        return nextFrameTime_ - curTime;
    else
        return duration_t::zero();
}

FrameTimer::duration_t FrameTimer::calcTimeToNextFrame(clock::time_point curTime, int& framesBehindSchedule) const
{
    if(targetFrameDuration_ == duration_t::zero())
    {
        framesBehindSchedule = 0;

        if(curTime - lastUpdateTime_ > std::chrono::milliseconds(33))
        {
            return lastUpdateTime_ - curTime;
        }
    } else
    {
        framesBehindSchedule = (curTime - nextFrameTime_) / targetFrameDuration_;
    }

    return calcTimeToNextFrame(curTime);
}

void FrameTimer::update(clock::time_point curTime)
{
    // Ideal: nextFrameTime == startTime -> Current frame is punctual
    // Normal: nextFrameTime + x == startTime; -targetFrameDuration < x < targetFrameDuration (1 frame early to 1 frame
    // late) Problem: The calculations can take long so every frame is late making nextFrameTime be further and further
    // behind current time So even when changing the target we will never catch up -> Limit the time difference
    if(lastUpdateTime_ + maxLagFrames_ * targetFrameDuration_ < curTime)
    {
        totalLagShift_ += curTime - nextFrameTime_;
        nextFrameTime_ = curTime;
    } else
    {
        // Set the time for the next frame exactly 1 frame after the last
        // This way if the current frame was late, the next will be early and vice versa
        nextFrameTime_ += targetFrameDuration_;
    }

    lastUpdateTime_ = curTime;
}

FrameScheduler::FrameScheduler() = default;

void ScheduledObject::setTargetFramerate(int targetFramerate)
{
    frameTimer_.setTargetFramerate(targetFramerate);
}

void ScheduledObject::update(clock::time_point curTime)
{
    frameTimer_.update(curTime);
}

void ScheduledObject::setTargetFrameDuration(duration_t targetDuration)
{
    frameTimer_.setTargetFrameDuration(targetDuration);
}

void ScheduledObject::executeAndUpdate(clock::time_point curTime)
{
    this->Execute();
    update(curTime);
}

void FrameScheduler::addScheduledTask(ScheduledObject* newTask, int targetFramerate)
{
    scheduledTasks_.push_back(newTask);
    newTask->setTargetFramerate(targetFramerate);
}

void FrameScheduler::addScheduledTask(ScheduledObject* newTask, duration_t targetFrameDuration)
{
    scheduledTasks_.push_back(newTask);
    newTask->setTargetFrameDuration(targetFrameDuration);
}

using namespace std::chrono;

struct schedulingTime
{
public:
    nanoseconds waitTime_;
    ScheduledObject* scheduledTask_;
    using clock = FrameTimer::clock;
    int framesBehindSchedule_;

    schedulingTime(ScheduledObject* scheduledTask, clock::time_point curTime) : scheduledTask_(scheduledTask)
    {
        waitTime_ = duration_cast<nanoseconds>(
          scheduledTask_->getFrameTimer().calcTimeToNextFrame(curTime, framesBehindSchedule_));
    }

    bool operator<(const schedulingTime& other) const
    {
        if(waitTime_ < other.waitTime_)
        {
            return true;
        } else if(waitTime_ == other.waitTime_)
        {
            return framesBehindSchedule_ > other.framesBehindSchedule_;
        } else
        {
            return false;
        }
    }
};

void FrameScheduler::sleepTillNextFrame(clock::time_point curTime)
{
    std::vector<schedulingTime> waitTime;
    for(auto& task : scheduledTasks_)
    {
        if(!task->IsPaused())
            waitTime.emplace_back(task, curTime);
    }

    auto min_it = std::min_element(waitTime.begin(), waitTime.end());

    // No time to waste?
    if(min_it->waitTime_ <= nanoseconds::zero())
        min_it->scheduledTask_->executeAndUpdate(curTime);
#ifdef _WIN32
    else if(waitTime
            < milliseconds(13)) // timer resolutions < 13ms do not work for windows correctly. TODO: Still true?
        min_it->scheduledTask_->executeAndUpdate(curTime);
#endif
    else
    {
        timespec req;
        req.tv_sec = static_cast<time_t>(duration_cast<seconds>(min_it->waitTime_).count());
        req.tv_nsec = static_cast<long>((min_it->waitTime_ - seconds(req.tv_sec)).count());

        while(nanosleep(&req, &req) == -1)
            continue;

        min_it->scheduledTask_->executeAndUpdate();
    }
}
