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

	if (!is_valid(argc, argv))
		return (printf("fail parameters\n"), 0);
	init_arguments(&args, argv);
	if (!init_sim(&sim, &args))
		return (printf("init failed\n"), 1);
	i = -1;
	while (++i < sim.config.nb_of_coders)
	{
		if (pthread_create(&sim.coders[i].th, NULL,
				coder_routine, &sim.coders[i]) != 0)
		{
			sim_set_stop(&sim);
			break ;
		}
	}
	if (pthread_create(&monitor_th, NULL, monitor_routine, &sim) != 0)
		sim_set_stop(&sim);
	while (--i >= 0)
		pthread_join(sim.coders[i].th, NULL);
	return (pthread_join(monitor_th, NULL), sim_destroy(&sim), 0);
}
