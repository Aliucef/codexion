/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stop.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:18:12 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:18:12 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STOP_H
# define STOP_H

# include "../parser/parse.h"

int		sim_should_stop(t_sim *sim);
void	sim_set_stop(t_sim *sim);

#endif
