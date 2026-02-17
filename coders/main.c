/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 11:42:11 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 11:53:52 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/parse.h"
#include "stdio.h"
#include "coders.h"
#include "stop/stop.h"
#include "logs/log.h"


void	sim_destroy(t_sim *sim)
{
	if (!sim)
		return ;
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
	{
		printf("fail parameters\n");
		return (0);
	}
	init_arguments(&args, argv);

	if (!init_sim(&sim, &args))
	{
		printf("init failed\n");
		return (1);
	}

	// start coder threads
	i = 0;
	while (i < sim.config.nb_of_coders)
	{
		if (pthread_create(&sim.coders[i].th, NULL, coder_routine, &sim.coders[i]) != 0)
		{
			sim_set_stop(&sim);
			break;
		}
		i++;
	}

	// start monitor thread
	if (pthread_create(&monitor_th, NULL, monitor_routine, &sim) != 0)
		sim_set_stop(&sim);

	// join coder threads (only those created)
	while (--i >= 0)
		pthread_join(sim.coders[i].th, NULL);

	// join monitor thread (only if created successfully is ideal; keep it simple for now)
	pthread_join(monitor_th, NULL);

	sim_destroy(&sim);
	return (0);
}
