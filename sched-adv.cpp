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
class Advancedcheduler : public SchedulingAlgorithm
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
    * Helper function for generating a random number using the "middle square" method.
    */
    int number_randomiser() 
    {
        int number = ((entities_counter * entities_counter) / 10) % 100;
        return number;
    }

    /**
    * Helper function for generating a random ticket number. Ensures the outputed random number
    * is within the given range of 1 to 100.
    */
    int ticket_number_generator() 
    {
        int min = 1;
        int max = 100;
        return number_randomiser() % ( (1 + max - min) + 1);
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

        // if there are entities to be scheduled, we use the Lottery Scheduling Algorithm
        else 
        {
            // create a pointer for the next entity to be scheduled
            SchedulingEntity* next_entity = NULL;
            // update the total number of entities scheduled
            entities_counter++;
            // generates a ticket number completely randomly
            int ticket_number = ticket_number_generator();
            // block interrupts
            UniqueIRQLock l;
            // based on the ticket number generated
            if (ticket_number >= 1 && ticket_number <= 40)
            {
                // try the queues in different order
                if (realtime_runqueue.count() > 0)
                {
                    next_entity = realtime_runqueue.dequeue();
                    realtime_runqueue.enqueue(next_entity);
                }
                else if (interactive_runqueue.count() > 0)
                {
                    next_entity = interactive_runqueue.dequeue();
                    interactive_runqueue.enqueue(next_entity);
                }
                else if (normal_runqueue.count() > 0)
                {
                    next_entity = normal_runqueue.dequeue();
                    normal_runqueue.enqueue(next_entity);
                }
                else
                {
                    next_entity = daemon_runqueue.dequeue();
                    daemon_runqueue.enqueue(next_entity);
                }
            }
            // then check interactive queue first
            else if (ticket_number >= 41 && ticket_number <= 70)
            {
                // try the queues in different order
                if (interactive_runqueue.count() > 0)
                {
                    next_entity = interactive_runqueue.dequeue();
                    interactive_runqueue.enqueue(next_entity);
                }
                else if (normal_runqueue.count() > 0) 
                {
                    next_entity = normal_runqueue.dequeue();
                    normal_runqueue.enqueue(next_entity);
                }
                else if (daemon_runqueue.count() > 0) 
                {
                    next_entity = daemon_runqueue.dequeue();
                    daemon_runqueue.enqueue(next_entity);
                }
                else 
                {
                    next_entity = realtime_runqueue.dequeue();
                    realtime_runqueue.enqueue(next_entity);
                }
            }
            // then normal queue first
            else if (ticket_number >= 71 && ticket_number <= 90)
            {
                // try the queues in different order
                if (normal_runqueue.count() > 0)
                {
                    next_entity = normal_runqueue.dequeue();
                    normal_runqueue.enqueue(next_entity);
                }
                else if (daemon_runqueue.count() > 0) 
                {
                    next_entity = daemon_runqueue.dequeue();
                    daemon_runqueue.enqueue(next_entity);
                }
                else if (realtime_runqueue.count() > 0)
                {
                    next_entity = realtime_runqueue.dequeue();
                    realtime_runqueue.enqueue(next_entity);
                }
                else 
                {
                    next_entity = interactive_runqueue.dequeue();
                    interactive_runqueue.enqueue(next_entity);
                }
            }
            // lastly check daemon queue first
            else 
            {
                // try the queues in different order
                if (daemon_runqueue.count() > 0) 
                {
                    next_entity = daemon_runqueue.dequeue();
                    daemon_runqueue.enqueue(next_entity);
                }
                else if (realtime_runqueue.count() > 0) 
                {
                    next_entity = realtime_runqueue.dequeue();
                    realtime_runqueue.enqueue(next_entity);
                }
                else if (interactive_runqueue.count() > 0) 
                {
                    next_entity = interactive_runqueue.dequeue();
                    interactive_runqueue.enqueue(next_entity);
                }
                else 
                {
                    next_entity = normal_runqueue.dequeue();
                    normal_runqueue.enqueue(next_entity);
                }
            }
            // and return the selected entity
            return next_entity;
        }
    }
// queues for each priority level   
private:
	List<SchedulingEntity*> realtime_runqueue;
	List<SchedulingEntity*> interactive_runqueue;
	List<SchedulingEntity*> normal_runqueue;
	List<SchedulingEntity*> daemon_runqueue;
    // keep track of all the entities scheduled so far
    int entities_counter = 0;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(Advancedcheduler);
