/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_simulator.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 12:42:43 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:05:30 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <string.h>
#include "../validation/validate.h"
#include <sys/time.h>

static int	sim_init_cleanup(t_sim *sim, int coders_inited, int dongles_inited)
{
	int	k;

	k = 0;
	while (k < dongles_inited)
	{
		pthread_cond_destroy(&sim->dongles[k].condvar);
		pthread_mutex_destroy(&sim->dongles[k].mutex);
		k++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
	k = -1;
	while (++k < coders_inited)
		pthread_mutex_destroy(&sim->coders[k].m);
	free(sim->coders);
	sim->coders = NULL;
	pthread_mutex_destroy(&sim->log_m);
	pthread_mutex_destroy(&sim->stop_m);
	return (0);
}

static int	init_coders(t_sim *sim, int *out_inited)
{
	int		i;
	t_coder	*c;

	*out_inited = 0;
	sim->coders = malloc(sizeof(t_coder) * sim->config.nb_of_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->config.nb_of_coders)
	{
		c = &sim->coders[i];
		memset(c, 0, sizeof(*c));
		c->id = i + 1;
		c->sim = sim;
		c->last_compile_start_ms = sim->start_ms;
		if (pthread_mutex_init(&c->m, NULL) != 0)
			return (0);
		(*out_inited)++;
		i++;
	}
	return (1);
}

static int	init_dongles(t_sim *sim, int *out_inited)
{
	int			i;
	t_dongle	*d;

	*out_inited = 0;
	sim->dongles = malloc(sizeof(t_dongle) * sim->config.nb_of_coders);
	if (!sim->dongles)
		return (0);
	i = -1;
	while (++i < sim->config.nb_of_coders)
	{
		d = &sim->dongles[i];
		memset(d, 0, sizeof(*d));
		d->held = 0;
		d->cooldown_until_ms = sim->start_ms;
		if (pthread_mutex_init(&d->mutex, NULL) != 0)
			return (0);
		if (pthread_cond_init(&d->condvar, NULL) != 0)
			return (pthread_mutex_destroy(&d->mutex), 0);
		(*out_inited)++;
	}
	return (1);
}

static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_m, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->log_m, NULL) != 0)
		return (pthread_mutex_destroy(&sim->stop_m), 0);
	return (1);
}

int	init_sim(t_sim *sim, const t_parse *config)
{
	int	coders_inited;
	int	dongles_inited;

	if (!sim || !config || config->nb_of_coders <= 0)
		return (0);
	memset(sim, 0, sizeof(*sim));
	sim->config = *config;
	sim->start_ms = get_time_ms();
	sim->stop = 0;
	if (!init_mutexes(sim))
		return (0);
	if (!init_coders(sim, &coders_inited))
		return (sim_init_cleanup(sim, coders_inited, 0));
	if (!init_dongles(sim, &dongles_inited))
		return (sim_init_cleanup(sim, coders_inited, dongles_inited));
	return (1);
}
