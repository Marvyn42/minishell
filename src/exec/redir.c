/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agautier <agautier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/24 20:51:00 by agautier          #+#    #+#             */
/*   Updated: 2021/05/26 16:1738 by agautier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
**	
*/
static char	redir_open(t_token *token, int (*redirs)[4], int type, t_err *err)
{
	struct stat	buf;
	int			ret;

	ret = stat(token->data[1], &buf);
	if (token->data[0][0] == '<')
	{
		if (ret != 0)
			return ((long)error(err, NO_SUCH_FILE, NULL, NULL));
		if (!(buf.st_mode & S_IRUSR))
			return ((long)error(err, PERM, NULL, NULL));
		(*redirs)[type] = open(token->data[1], O_RDONLY);
		return (SUCCESS);
	}
	if (ret != -1 && buf.st_mode & S_IFDIR)
		return ((long)error(err, REDIR_DIR, NULL, NULL));
	if (ret != -1 && !(buf.st_mode & S_IWUSR))
		return ((long)error(err, PERM, NULL, NULL));
	if (token->data[0][1] == '>')
		(*redirs)[type] = open(token->data[1],
				O_WRONLY | O_APPEND | O_CREAT, 0664);
	else
		(*redirs)[type] = open(token->data[1],
				O_WRONLY | O_TRUNC | O_CREAT, 0664);
	return (SUCCESS);
}

/*
**	
*/
char	redir_init(t_token *token, int (*redirs)[4], t_err *err)
{
	int	type;

	err->cmd_name = gc_strdup(token->data[1], err->gc);
	if (!(err->cmd_name))
		return ((long)error(err, FATAL, NULL, NULL));
	type = IN;
	if (token->data[0][0] == '>')
		type = OUT;
	if (!redir_destroy(type, redirs, err))
		return (FAILURE);
	(*redirs)[REAL + type] = dup(type);
	if ((*redirs)[REAL + type] == -1)
		return ((long)error(err, FATAL, NULL, NULL));
	if (!redir_open(token, redirs, type, err))
		return (FAILURE);
	if ((*redirs)[type] == -1)
		return ((long)error(err, NO_SUCH_FILE, NULL, NULL));
	if (dup2((*redirs)[type], type) == -1)
		return ((long)error(err, FATAL, NULL, NULL));
	return (SUCCESS);
}

/*
**	
*/
char	redir_destroy(int type, int (*redirs)[4], t_err *err)
{
	if ((*redirs)[REAL + type] != -1)
	{
		if (dup2((*redirs)[REAL + type], type) == -1)
			return ((long)error(err, FATAL, NULL, NULL));
		(*redirs)[REAL + type] = -1;
	}
	if ((*redirs)[type] != type)
	{
		if (close((*redirs)[type]) == -1)
			return ((long)error(err, FATAL, NULL, NULL));
		(*redirs)[type] = type;
	}
	return (SUCCESS);
}
