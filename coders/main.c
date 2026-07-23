/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:42:11 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 14:05:51 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/parse.h" // t_sim, t_parse, init_sim(), init_arguments(), get_time_ms()
#include "stdio.h"        // printf()
#include "coders.h"       // coder_routine(), monitor_routine(), t_coder, t_dongle
#include "stop/stop.h"    // sim_set_stop()
#include "logs/log.h"     // log_state() — used indirectly
#include <stdlib.h>       // free()

// Frees all heap-allocated simulator resources and destroys all mutexes/condvars.
// Called once, after all threads have been joined (so no concurrent access possible).
// Order matters: destroy per-coder resources first, then per-dongle, then shared mutexes.
void	sim_destroy(t_sim *sim)
{
	int	i; // loop index

	if (!sim)  // safety: if sim pointer is null, nothing to free
		return ;
	i = -1;
	while (++i < sim->config.nb_of_coders) // e.g. 5 coders → i = 0,1,2,3,4
	{
		pthread_cond_destroy(&sim->coders[i].wait_cond); // destroy per-coder condvar, e.g. coders[0].wait_cond
		pthread_mutex_destroy(&sim->coders[i].m);        // destroy per-coder mutex, e.g. coders[0].m
	}
	free(sim->coders); // free the entire coders array (malloc'd in init_coders)
	i = -1;
	while (++i < sim->config.nb_of_coders) // e.g. 5 dongles → i = 0,1,2,3,4
	{
		free(sim->dongles[i].queue);                  // free the priority-queue array inside each dongle
		pthread_mutex_destroy(&sim->dongles[i].mutex); // destroy dongle mutex
	}
	free(sim->dongles);               // free the dongles array (malloc'd in init_dongles)
	pthread_mutex_destroy(&sim->log_m);  // destroy shared log mutex (serializes printf)
	pthread_mutex_destroy(&sim->stop_m); // destroy shared stop mutex (guards sim->stop flag)
}

// Program entry point.
// Validates args, initializes the simulator, spawns N coder threads + 1 monitor thread,
// waits for all to finish, then cleans up.
// Example invocation: ./codexion 5 800 200 200 200 3 50 fifo
int	main(int argc, char **argv)
{
	t_parse		args;       // parsed config from argv, e.g. {nb_of_coders=5, time_to_burnout=800, ...}
	t_sim		sim;        // the entire simulator state: coders[], dongles[], mutexes, stop flag
	pthread_t	monitor_th; // handle for the single monitor thread
	int			i;          // loop index for thread creation/join

	if (!is_valid(argc, argv))             // checks argc==9, all args are digits, arg8 is "fifo"/"edf"
		return (printf("fail parameters\n"), 0); // e.g. wrong number of args → print error, exit 0
	init_arguments(&args, argv);           // parse argv[1..8] into the t_parse struct
	if (!init_sim(&sim, &args))            // malloc coders/dongles, init all mutexes and condvars
		return (printf("init failed\n"), 1); // e.g. malloc failed → print error, exit 1
	i = -1;
	while (++i < sim.config.nb_of_coders) // create one thread per coder, e.g. 5 threads
	{
		if (pthread_create(&sim.coders[i].th, NULL,
				coder_routine, &sim.coders[i]) != 0) // pass each coder's own t_coder as arg
		{
			sim_set_stop(&sim); // thread creation failed → tell everyone to stop
			break ;             // stop creating more threads (already-created ones will exit)
		}
	}
	if (pthread_create(&monitor_th, NULL, monitor_routine, &sim) != 0) // create the monitor thread
		sim_set_stop(&sim); // monitor creation failed → signal stop so coders can exit
	while (--i >= 0)        // join all successfully created coder threads (i decrements from n-1 to 0)
		pthread_join(sim.coders[i].th, NULL); // wait for coder i to exit, e.g. coder 5, 4, 3, 2, 1
	return (pthread_join(monitor_th, NULL), sim_destroy(&sim), 0);
	// join monitor → wait for it to finish → then destroy all resources → exit 0
}
