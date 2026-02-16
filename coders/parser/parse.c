/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 09:15:39 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 17:13:45 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <string.h>
#include "../validation/validate.h"


void	init_arguments(t_parse *args, char **argv)
{
	args->nb_of_coders = atoi(argv[1]);
	args->time_to_burnout = atoi(argv[2]);
	args->time_to_compile = atoi(argv[3]);
	args->time_to_debug = atoi(argv[4]);
	args->time_to_refactor = atoi(argv[5]);
	args->required_compiles = atoi(argv[6]);
	args->dongle_cooldown = atoi(argv[7]);
	if (!parse_scheduler(argv[8], &args->scheduler))
		return ;
}


int	get_time_ms()
{
	return 0;
}


static int	sim_init_cleanup(t_sim *sim, int coders_inited, int dongles_inited)
{
	int k;

	k = 0;
	while (k < dongles_inited)
	{
		pthread_cond_destroy(&sim->dongles[k].condvar);
		pthread_mutex_destroy(&sim->dongles[k].mutex);
		k++;
	}
	free(sim->dongles);
	sim->dongles = NULL;

	k = 0;
	while (k < coders_inited)
	{
		pthread_mutex_destroy(&sim->coders[k].m);
		k++;
	}
	free(sim->coders);
	sim->coders = NULL;

	pthread_mutex_destroy(&sim->log_m);
	pthread_mutex_destroy(&sim->stop_m);
	return (0);
}

int	init_sim(t_sim *sim, const t_parse *config)
{
	int			i;
	t_coder		*c;
	t_dongle	*d;
	i = 0;
	if (!sim || !config)
		return (0);
	if (config->nb_of_coders <= 0)
		return (0);
	memset(sim, 0, sizeof(*sim)); // preventing garbage later
	sim->config = *config;
	sim->start_ms = get_time_ms();
	sim->stop = 0;
	if (pthread_mutex_init(&sim->stop_m, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->log_m, NULL) != 0)
		return (pthread_mutex_destroy(&sim->stop_m), 0);
	sim->coders = malloc(sizeof(t_coder) * sim->config.nb_of_coders);
	if (!sim->coders)
		return (sim_init_cleanup(sim, 0, 0));
	while (i < sim->config.nb_of_coders)
	{
		c = &sim->coders[i];
		memset(c, 0, sizeof(*c));
		c->id = i + 1;
		c->sim = sim;
		c->compile_count = 0;
		c->last_compile_start_ms = sim->start_ms;

		if (pthread_mutex_init(&c->m, NULL) != 0)
			return (sim_init_cleanup(sim, i, 0));
		i++;
	}
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.nb_of_coders);
	if (!sim->dongles)
		return (sim_init_cleanup(sim, sim->config.nb_of_coders, 0));

	i = 0;
	while (i < sim->config.nb_of_coders)
	{
		d = &sim->dongles[i];
		memset(d, 0, sizeof(*d));
		d->held = 0;
		d->cooldown_until_ms = sim->start_ms;
		if (pthread_mutex_init(&d->mutex, NULL) != 0)
			return (sim_init_cleanup(sim, sim->config.nb_of_coders, i));
		if (pthread_cond_init(&d->condvar, NULL) != 0)
		{
			pthread_mutex_destroy(&d->mutex);
			return (sim_init_cleanup(sim, sim->config.nb_of_coders, i));
		}
		i++;
	}
	return (1);
}
