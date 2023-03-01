/*
 * The Priority Task Scheduler
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 1
 */

#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>

using namespace infos::kernel;
using namespace infos::util;

/**
 * A Multiple Queue priority scheduling algorithm
 */
class MultipleQueuePriorityScheduler : public SchedulingAlgorithm
{
public:
    /**
     * Returns the friendly name of the algorithm, for debugging and selection purposes.
     */
    const char* name() const override { return "mq"; }

    /**
     * Called during scheduler initialisation.
     */
    void init()
    {
        // initialise and partition the runqueue
        List<List<SchedulingEntity*>*> runqueue;
        runqueue.append(&realtime_runqueue);
        runqueue.append(&interactive_runqueue);
        runqueue.append(&normal_runqueue);
        runqueue.append(&daemon_runqueue);
    }

    /**
     * Called when a scheduling entity becomes eligible for running.
     * @param entity
     */
    void add_to_runqueue(SchedulingEntity& entity) override
    {
        // block interrupts
        UniqueIRQLock l;
        // add the entity to the correct queue based on its priority
        if (entity.priority() == SchedulingEntityPriority::REALTIME) 
        {
            realtime_runqueue.enqueue(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::INTERACTIVE) 
        {
            interactive_runqueue.enqueue(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::NORMAL) 
        {
            normal_runqueue.enqueue(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::DAEMON) 
        {
            daemon_runqueue.enqueue(&entity);
        }
        else 
        {
            // priority does not exist
        }
    }

    /**
     * Called when a scheduling entity is no longer eligible for running.
     * @param entity
     */
    void remove_from_runqueue(SchedulingEntity& entity) override
    {
        // block interrupts
        UniqueIRQLock l;
        // remove the entity from the correct queue based on its priority
        if (entity.priority() == SchedulingEntityPriority::REALTIME) 
        {
            realtime_runqueue.remove(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::INTERACTIVE) 
        {
            interactive_runqueue.remove(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::NORMAL) 
        {
            normal_runqueue.remove(&entity);
        }
        else if (entity.priority() == SchedulingEntityPriority::DAEMON) 
        {
            daemon_runqueue.remove(&entity);
        }
        else 
        {
            // priority does not exist
        }
    }

    /**
     * Helper function implementing the RR algorithm. This function is called in scenarios 
     * where there are multiple entities of the same priority in a queue.
     * @param queue
     */
    SchedulingEntity *round_robin_selector(List<SchedulingEntity*>& queue)
    {
        // block interrupts
        UniqueIRQLock l;
        // get first entity in the queue
        SchedulingEntity* next_entity = queue.dequeue();
        // if it is still active, add it to the end of the queue
        queue.enqueue(next_entity);
		return next_entity;
    }

    /**
     * Called every time a scheduling event occurs, to cause the next eligible entity
     * to be chosen.  The next eligible entity might actually be the same entity, if
     * e.g. its timeslice has not expired.
     */
    SchedulingEntity *pick_next_entity() override
    {
        // if all queues are empty, return null
        if (realtime_runqueue.count() == 0 && interactive_runqueue.count() == 0 
        && normal_runqueue.count() == 0 && daemon_runqueue.count() == 0) return NULL;

        // if the REALTIME queue is not empty
        // we first ensure it gets emptied before considering entities in other queues
        else if (realtime_runqueue.count() > 0) 
        {
            // if the queue only has one element, return that, otherwise perform RR scheduling
            if (realtime_runqueue.count() == 1) return realtime_runqueue.first();
            return round_robin_selector(realtime_runqueue);
        }

        // if the REALTIME queue is empty
        // we then ensure the INTERACTIVE queue is emptied (in case it is not empty already)
        else if (realtime_runqueue.count() == 0 && interactive_runqueue.count() > 0) 
        {
            // if the queue only has one element, return that, otherwise perform RR scheduling
            if (interactive_runqueue.count() == 1) return interactive_runqueue.first();
            return round_robin_selector(interactive_runqueue);
        }

        // if the REALTIME and INTERACTIVE queues are empty
        // we then ensure the NORMAL queue is emptied (in case it is not empty already)
        else if (realtime_runqueue.count() == 0 && interactive_runqueue.count() == 0 
        && normal_runqueue.count() > 0) 
        {
            // if the queue only has one element, return that, otherwise perform RR scheduling
            if (normal_runqueue.count() == 1) return normal_runqueue.first();
            return round_robin_selector(normal_runqueue);
        }

        // otherwise we ensure the DAEMON queue is emptied
        else
        {
            // if the queue only has one element, return that, otherwise perform RR scheduling
            if (daemon_runqueue.count() == 1) return daemon_runqueue.first();
            return round_robin_selector(daemon_runqueue);
        } 
    }
    
private:
	List<SchedulingEntity*> realtime_runqueue;
	List<SchedulingEntity*> interactive_runqueue;
	List<SchedulingEntity*> normal_runqueue;
	List<SchedulingEntity*> daemon_runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(MultipleQueuePriorityScheduler);
