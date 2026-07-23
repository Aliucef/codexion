/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:54:27 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/23 11:33:45 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"
#include "parser/parse.h"
#include <unistd.h>
#include "logs/log.h"
#include "stop/stop.h"
#include <unistd.h>
#include "dongles/dongles.h"

static int	get_compile_count(t_coder *c) // gets compile count for each coder (what is left to compile)
{
	int	v;

	pthread_mutex_lock(&c->m); // locks in soo no other coder interrupt
	v = c->compile_count;
	pthread_mutex_unlock(&c->m);
	return (v);
}

void	sim_wake_all(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->config.nb_of_coders)
		pthread_cond_signal(&sim->coders[i].wait_cond); // finish whats left
}

static void	coder_do_cycle(t_coder *coder, int first, int second)
{
	long	now;

	if (!dongle_take(coder->sim, first, coder->id))
		return ;
	if (second != first && !dongle_take(coder->sim, second, coder->id))
		return (dongle_release(coder->sim, first), (void)0);
	now = get_time_ms(); // now time
	pthread_mutex_lock(&coder->m); // lock coder
	coder->last_compile_start_ms = now; // set coder last compile to now
	pthread_mutex_unlock(&coder->m); // unlock
	log_state(coder->sim, coder->id, "is compiling"); // print
	usleep(coder->sim->config.time_to_compile * 1000);// break
	dongle_release(coder->sim, first); // release the first dongle
	if (second != first)
		dongle_release(coder->sim, second); //release the second dongle
	pthread_mutex_lock(&coder->m); // lock the coder
	coder->compile_count++; // increment its compiling count
	pthread_mutex_unlock(&coder->m); // unlock
	log_state(coder->sim, coder->id, "is debugging"); // print
	usleep(coder->sim->config.time_to_debug * 1000); // give it debug time
	log_state(coder->sim, coder->id, "is refactoring"); // prin
	usleep(coder->sim->config.time_to_refactor * 1000); // give it refactoring time
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		n;
	int		first;
	int		second;

	coder = (t_coder *)arg; // cating the argument which is void , to t_coder
	n = coder->sim->config.nb_of_coders; // n represents the total number of coders
	if (coder->id - 1 < coder->id % n) //
	{
		first = coder->id - 1;
		second = coder->id % n;
	}
	else
	{
		first = coder->id % n;
		second = coder->id - 1;
	}
	if (n == 1)
		second = first;
	while (!sim_should_stop(coder->sim)
		&& get_compile_count(coder) < coder->sim->config.required_compiles)
		coder_do_cycle(coder, first, second);
	return (NULL);
}
