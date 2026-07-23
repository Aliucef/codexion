/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:24:39 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include "logs/log.h"
#include "stop/stop.h"
#include "dongles/dongles.h"
#include <time.h>

void	ft_usleep(t_sim *sim, long ms)
{
	long			end_ms;
	struct timespec	ts;
	pthread_mutex_t	m;
	pthread_cond_t	c;

	end_ms = get_time_ms() + ms;
	ts.tv_sec = end_ms / 1000;
	ts.tv_nsec = (end_ms % 1000) * 1000000L;
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
	pthread_mutex_lock(&m);
	while (!sim_should_stop(sim) && get_time_ms() < end_ms)
		pthread_cond_timedwait(&c, &m, &ts);
	pthread_mutex_unlock(&m);
	pthread_cond_destroy(&c);
	pthread_mutex_destroy(&m);
}

static int	get_compile_count(t_coder *c)
{
	int	v;

	pthread_mutex_lock(&c->m);
	v = c->compile_count;
	pthread_mutex_unlock(&c->m);
	return (v);
}

void	sim_wake_all(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
		pthread_cond_signal(&sim->coders[i].wait_cond);
}

static void	coder_do_cycle(t_coder *coder, int first, int second)
{
	long	now;

	if (!dongle_take(coder->sim, first, coder->id))
		return ;
	if (second == first || !dongle_take(coder->sim, second, coder->id))
		return (dongle_release(coder->sim, first), (void)0);
	now = get_time_ms();
	pthread_mutex_lock(&coder->m);
	coder->last_compile_start_ms = now;
	pthread_mutex_unlock(&coder->m);
	log_state(coder->sim, coder->id, "is compiling");
	ft_usleep(coder->sim, coder->sim->config.time_to_compile);
	dongle_release(coder->sim, first);
	if (second != first)
		dongle_release(coder->sim, second);
	pthread_mutex_lock(&coder->m);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->m);
	log_state(coder->sim, coder->id, "is debugging");
	ft_usleep(coder->sim, coder->sim->config.time_to_debug);
	log_state(coder->sim, coder->id, "is refactoring");
	ft_usleep(coder->sim, coder->sim->config.time_to_refactor);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		n;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	n = coder->sim->config.nb_of_coders;
	if (coder->id - 1 < coder->id % n)
	{
		first = coder->id - 1;
		second = coder->id % n;
	}
	else
	{
		first = coder->id % n;
		second = coder->id - 1;
	}
	while (!sim_should_stop(coder->sim)
		&& get_compile_count(coder) < coder->sim->config.required_compiles)
		coder_do_cycle(coder, first, second);
	return (NULL);
}
