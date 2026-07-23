/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:40:03 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/23 09:14:09 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H

# include <pthread.h>

typedef struct s_sim	t_sim; //simulator?

typedef struct coders // coders
{
	int					id; // coder id
	pthread_t			th; // coder thread
	int					compile_count; // compile count
	long				last_compile_start_ms; // save time
	pthread_mutex_t		m; // mutex
	pthread_cond_t		wait_cond; // conditional wait
	struct s_sim		*sim; // the simulator

}	t_coder;

typedef struct s_waiter
{
	int		coder_id; // coder id
	long	priority; // priority depending on the algorithm
}	t_waiter;

typedef struct s_dongle
{
	pthread_mutex_t	mutex; // a mutex to lock each dongle
	int				held; // boolean indicatin if the dongle is held
	long			cooldown_until_ms; // cool down for each dongle
	t_waiter		*queue; // who is waiting for the dongle in line , (which coder?)
	int				queue_size; // number of waiters
}	t_dongle;

void	sim_wake_all(t_sim *sim);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

#endif
