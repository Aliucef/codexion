/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:42:11 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/23 11:30:55 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/parse.h"
#include "stdio.h"
#include "coders.h"
#include "stop/stop.h"
#include "logs/log.h"
#include <stdlib.h>

void	sim_destroy(t_sim *sim)
{
	int	i;

	if (!sim)
		return ;
	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		pthread_cond_destroy(&sim->coders[i].wait_cond);
		pthread_mutex_destroy(&sim->coders[i].m);
	}
	free(sim->coders);
	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		free(sim->dongles[i].queue);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
	}
	free(sim->dongles);
	pthread_mutex_destroy(&sim->log_m);
	pthread_mutex_destroy(&sim->stop_m);
}

int	main(int argc, char **argv)
{
	t_parse		args;
	t_sim		sim;
	pthread_t	monitor_th;
	int			i;

	if (!is_valid(argc, argv)) //validating arguments
		return (printf("fail parameters\n"), 0);
	init_arguments(&args, argv); // initialize arguments to their input value after sanitizing
	if (!init_sim(&sim, &args)) // starts a simulator
		return (printf("init failed\n"), 1); // on failure
	i = -1;
	while (++i < sim.config.nb_of_coders) // loop on each coder
	{
		if (pthread_create(&sim.coders[i].th, NULL, //creating a thread for each coder , with the coder_routine function assigned, 1st parameter : the variable where the id of that new thread is gonna be assigned,
				coder_routine, &sim.coders[i]) != 0) // the coder routine does the whole funcionality
		{
			sim_set_stop(&sim); // if it returns anything but 0 this breaks the loop and the simulator
			break ;
		}
	}
	if (pthread_create(&monitor_th, NULL, monitor_routine, &sim) != 0) // create a new thread to monitor the
		sim_set_stop(&sim);
	while (--i >= 0)
		pthread_join(sim.coders[i].th, NULL); // join threads back in main
	return (pthread_join(monitor_th, NULL), sim_destroy(&sim), 0); //
}
