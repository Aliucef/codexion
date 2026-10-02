/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:40:03 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 12:28:58 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H

# include <pthread.h>

typedef struct s_sim	t_sim;

typedef struct coders
{
	int					id;
	pthread_t			th;
	int					compile_count;
	long				last_compile_start_ms;
	pthread_mutex_t		m;
	pthread_cond_t		wait_cond;
	struct s_sim		*sim;

}	t_coder;

typedef struct s_waiter
{
	int		coder_id;
	long	priority;
}	t_waiter;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				held;
	long			cooldown_until_ms;
	t_waiter		*queue;
	int				queue_size;
}	t_dongle;

void	ft_usleep(t_sim *sim, long ms);
void	sim_wake_all(t_sim *sim);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);

#endif
