/*
 * The Priority Task Scheduler Advanced Task
 */

#include <infos/kernel/sched.h>
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
class AdvancedScheduler : public SchedulingAlgorithm
{
public:
    /**
     * Returns the friendly name of the algorithm, for debugging and selection purposes.
     */
    const char* name() const override { return "adv"; }

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
     * Helper function implementing the next entity selection algorithm.
     * @param queue
     */
    SchedulingEntity *next_entity_selector(List<SchedulingEntity*>& queue)
    {
        // block interrupts
        UniqueIRQLock l;
        
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
            return next_entitys_selector(realtime_runqueue);
        }

        // if the REALTIME queue is empty
        // we then ensure the INTERACTIVE queue is emptied (in case it is not empty already)
        else if (realtime_runqueue.count() == 0 && interactive_runqueue.count() > 0) 
        {
            return next_entity_selector(interactive_runqueue);
        }

        // if the REALTIME and INTERACTIVE queues are empty
        // we then ensure the NORMAL queue is emptied (in case it is not empty already)
        else if (realtime_runqueue.count() == 0 && interactive_runqueue.count() == 0 
        && normal_runqueue.count() > 0) 
        {
            return next_entity_selector(normal_runqueue);
        }

        // otherwise we ensure the DAEMON queue is emptied
        else
        {
            return next_entity_selector(daemon_runqueue);
        }
    }

private:
	List<SchedulingEntity*> realtime_runqueue;
private:
	List<SchedulingEntity*> interactive_runqueue;
private:
	List<SchedulingEntity*> normal_runqueue;
private:
	List<SchedulingEntity*> daemon_runqueue;

// TODO: add a struct that keeps track of number of turns used

};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(AdvancedScheduler);
