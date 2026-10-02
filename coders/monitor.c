/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 00:00:00 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/22 00:00:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include <stdio.h>
#include "stop/stop.h"

static int	all_done(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].m);
		if (sim->coders[i].compile_count < sim->config.required_compiles)
			return (pthread_mutex_unlock(&sim->coders[i].m), 0);
		pthread_mutex_unlock(&sim->coders[i].m);
	}
	return (1);
}

static int	check_burnout(t_sim *sim, long now)
{
	long	last_start;
	int		i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].m);
		last_start = sim->coders[i].last_compile_start_ms;
		pthread_mutex_unlock(&sim->coders[i].m);
		if (now - last_start > sim->config.time_to_burnout)
			return (sim->coders[i].id);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		burned_id;

	sim = (t_sim *)arg;
	while (!sim_should_stop(sim))
	{
		burned_id = check_burnout(sim, get_time_ms());
		if (burned_id)
		{
			pthread_mutex_lock(&sim->log_m);
			printf("%ld %d burned out\n",
				get_time_ms() - sim->start_ms, burned_id);
			sim_set_stop(sim);
			pthread_mutex_unlock(&sim->log_m);
			return (sim_wake_all(sim), NULL);
		}
		if (all_done(sim))
			return (sim_set_stop(sim), sim_wake_all(sim), NULL);
		ft_usleep(sim, 1);
	}
	return (NULL);
}
