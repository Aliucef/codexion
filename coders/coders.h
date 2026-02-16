/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 13:40:03 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 17:01:03 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODERS_H
# define CODERS_H

#include <pthread.h>

typedef struct coders
{
	int					id;
	pthread_t			th;
	int					compile_count; // store progress , stops when a coder has compiled the required compiles
	long				last_compile_start_ms; // burnout watch later : last compile start ms = sim->start_ms

	pthread_mutex_t		m; // mutex to protect compile_count and last compile start ms, why? because the coder thread writes them the monitor reads them -> read write concurrency -> data race without a lock
	struct s_sim		*sim; // back-pointer to access whatever data inside it

}	t_coder;


typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	condvar;
	int				held;
	long			cooldown_until_ms;
}	t_dongle;



#endif
