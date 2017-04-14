#include <stdio.h>
#include <stdlib.h>

#include "path.h"

/* Create a new path */
path_t *path_new(void)
{
    path_t *p_new = malloc(sizeof *p_new);

    if (p_new != NULL)
    {
	p_new->length = 0;
	p_new->p_head = NULL;
	p_new->p_tail = NULL;
    }

    return p_new;
}

/* Add an action point to the end of the list */
path_t *path_append(path_t *p_list, pose_t pose, int (*callback)(void *data))
{
    if (p_list != NULL)
    {
	actionpoint_t *p_new = malloc(sizeof *p_new);
	if (p_new != NULL)
	{
	    p_new->pose = pose;
	    p_new->callback = callback;
	    p_new->p_next = NULL;
	    if (p_list->p_tail == NULL)
	    {
		p_new->p_prev = NULL;
		p_list->p_head = p_new;
		p_list->p_tail = p_new;
	    }
	    else
	    {
		p_list->p_tail->p_next = p_new;
		p_new->p_prev = p_list->p_tail;
		p_list->p_tail = p_new;
	    }
	    p_list->length++;
	}
    }

    return p_list;
}

/* Add an action point to the beginning of the list */
path_t *path_prepend(path_t *p_list, pose_t pose, int (*callback)(void *data))
{
    if (p_list != NULL)
    {
	actionpoint_t *p_new = malloc(sizeof *p_new);
	if (p_new != NULL)
	{
	    p_new->pose = pose;
	    p_new->callback = callback;
	    p_new->p_prev = NULL;
	    if (p_list->p_head == NULL)
	    {
		p_new->p_next = NULL;
		p_list->p_head = p_new;
		p_list->p_tail = p_new;
	    }
	    else
	    {
		p_list->p_head->p_prev = p_new;
		p_new->p_next = p_list->p_head;
		p_list->p_head = p_new;
	    }
	    p_list->length++;
	}
    }

    return p_list;
}

/* Display path */
void path_display(path_t *path)
{
    if (path!= NULL)
    {
	printf("START\n");

	actionpoint_t *p_temp = path->p_head;
	while (p_temp != NULL)
	{
	    printf("-> (x=%.01f, y=%.01f, O=%.01f), callback=%p\n", p_temp->pose.x, p_temp->pose.y, p_temp->pose.O,
		p_temp->callback);
	    p_temp = p_temp->p_next;
	}
    }
    printf("END\n");
}
