/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 09:30:13 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/16 15:03:00 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_H
# define PARSE_H

# include "../validation/validate.h"
# include <pthread.h>
# include "coders.h"


typedef struct s_parse
{
	int		nb_of_coders;
	int		time_to_burnout;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int		required_compiles;
	int		dongle_cooldown;
	t_sched	scheduler;
}	t_parse;

typedef struct s_sim
{
	t_parse			config;
	long			start_ms;
	int				stop;
	pthread_mutex_t	stop_m;
	pthread_mutex_t	log_m;
	t_coder			*coders;
	t_dongle		*dongles;

}	t_sim;




void	init_arguments(t_parse *args, char **argv);
int		init_sim(t_sim *sim, const t_parse *config);

#endif
