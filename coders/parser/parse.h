/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 09:30:13 by alyousse          #+#    #+#             */
/*   Updated: 2026/07/23 08:45:24 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_H
# define PARSE_H

# include "../validation/validate.h"
# include <pthread.h>
# include "../coders.h"

typedef struct s_parse // struct to parse in inputs
{
	int		nb_of_coders; // arg 1 : nb_of_coders
	int		time_to_burnout; // arg 2 : time to burnout
	int		time_to_compile; // arg 3 : time to compile
	int		time_to_debug; // arg 4 : time to debug
	int		time_to_refactor; // arg 5 : time to refactor
	int		required_compiles; // arg 6 : required compiles number
	int		dongle_cooldown; // arg 7 : dongle_cooldown
	t_sched	scheduler; // arg 8 : the scheduler (the chosen algorithm)
}	t_parse;

typedef struct s_sim
{
	t_parse			config; // all arguments lays in here
	long			start_ms; // save the start time of the simulation
	int				stop; // i guess this is a boolean tells it to start or stop
	pthread_mutex_t	stop_m; // initializing a mutex (mutual exclusion lock)
	pthread_mutex_t	log_m; // another lock for the logs so texts does not get interrupted by each others
	t_coder			*coders; // coder struct
	t_dongle		*dongles; // dongle struct

}	t_sim;

void	init_arguments(t_parse *args, char **argv);
int		init_sim(t_sim *sim, const t_parse *config);
long	get_time_ms(void);

#endif
