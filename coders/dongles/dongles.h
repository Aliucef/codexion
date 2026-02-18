/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alyousse <alyousse@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 13:22:41 by alyousse          #+#    #+#             */
/*   Updated: 2026/02/18 13:22:44 by alyousse         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DONGLES_H
# define DONGLES_H

int		dongle_take(t_sim *sim, int dongle_idx, int coder_id);
void	dongle_release(t_sim *sim, int dongle_idx);

#endif
