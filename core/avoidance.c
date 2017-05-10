#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include "odometry.h"
#include "avoidance.h"

/* Obstacle list. Each obstacle is a polygon */
static polygon_t polygons[POLY_MAX];
/* Number of polygons */
static int nb_polygons = 0;

/* Special polygon used as a simple list of visible points */
static polygon_t valid_points;

static double graph[GRAPH_MAX_VERTICES][GRAPH_MAX_VERTICES];

int avoidance(const pose_t *start, const pose_t *finish)
{
	/* Init all obstacles */
	init_polygons();

	/* Check that start and destination point are not in a polygon */
	for (int j = 0; j < nb_polygons; j++)
	{
		if ((start == NULL) || (finish == NULL)
			||(is_point_in_polygon(&polygons[j], *start) == true)
			|| (is_point_in_polygon(&polygons[j], *finish) == true))
		{
			return -1;
		}
	}

	valid_points.count = 0;
	if (valid_points.points)
		free(valid_points.points);
	valid_points.points = (pose_t *)malloc(2*sizeof(pose_t));
	valid_points.points[valid_points.count++] = *start;
	valid_points.points[valid_points.count++] = *finish;

	/* Build path graph */
	build_avoidance_graph();

	return 0;
}

/* Add a polygon to obstacle list */
int add_polygon(polygon_t *polygon)
{
	if (nb_polygons < POLY_MAX)
	{
		polygons[nb_polygons++] = *polygon;
		return 0;
	}
	else
	{
		return -1;
	}
}

/* Init all known fixed obstacles on map */
/* TODO: This should be done in machine code */
void init_polygons(void)
{
	polygon_t polygon;
	uint8_t nb_vertices;

	polygon.count = 0;
	nb_vertices = 6;
	if (nb_vertices < POLY_MAX_POINTS)
	{
		polygon.points = (pose_t *)malloc(nb_vertices * sizeof(pose_t));
		polygon.points[polygon.count++] = (pose_t){.x = 25, .y = 45};
		polygon.points[polygon.count++] = (pose_t){.x = 37.5, .y = 35};
		polygon.points[polygon.count++] = (pose_t){.x = 50, .y = 45};
		polygon.points[polygon.count++] = (pose_t){.x = 50, .y = 50};
		polygon.points[polygon.count++] = (pose_t){.x = 37.5, .y = 52};
		polygon.points[polygon.count++] = (pose_t){.x = 25, .y = 50};
		add_polygon(&polygon);
	}

	polygon.count = 0;
	nb_vertices = 4;
	if (nb_vertices < POLY_MAX_POINTS)
	{
		polygon.points = (pose_t *)malloc(nb_vertices * sizeof(pose_t));
		polygon.points[polygon.count++] = (pose_t){.x = 20, .y = 20};
		polygon.points[polygon.count++] = (pose_t){.x = 60, .y = 20};
		polygon.points[polygon.count++] = (pose_t){.x = 60, .y = 40};
		polygon.points[polygon.count++] = (pose_t){.x = 20, .y = 40};
		add_polygon(&polygon);
	}
}

/* Build obstacle graph
 * Each obstacle is a polygon.
 * List all  visible points : all points not contained in a polygon */
void build_avoidance_graph(void)
{
	/* For each polygon */
	for (int i = 0; i < nb_polygons; i++)
	{
		/* and for each vertice of that polygon */
		for (int p = 0; p < polygons[i].count; p++)
		{
			bool collide = 0;
			/* we check thios vertice is not inside an other polygon */
			for (int j = 0; j < nb_polygons; j++)
			{
				if (i == j)
				{
					continue;
				}
				if (is_point_in_polygon(&polygons[j], polygons[i].points[p]) == true)
				{
					collide++;
					break;
				}
			}
			/* If that point is not in an other polygon, add it to the list of valid points */
			if (collide == 0)
			{
				valid_points.points = (pose_t *)realloc(valid_points.points, (valid_points.count+1)*sizeof(pose_t));
				valid_points.points[valid_points.count++] = polygons[i].points[p];
			}
		}
	}

	/* For each segment of the valid points list */
	for (int p = 0; p < valid_points.count; p++)
	{
		for (int p2 = p+1; p2 < valid_points.count; p2++)
		{
			bool collide = 0;
			if (p != p2)
			{
				/* Check if that segment crosses a polygon */
				for (int i = 0; i < nb_polygons; i++)
				{
					for (int v = 0; v < polygons[i].count; v++)
					{
						pose_t p_next = ( (v + 1 == polygons[i].count) ? polygons[i].points[0] : polygons[i].points[v + 1]);

						if (is_segment_crossing_segment(valid_points.points[p],valid_points.points[p2], polygons[i].points[v], p_next) == true)
						{
							collide++;
							break;
						}
						/* Special case of internal crossing of a polygon */
						int8_t index = get_point_index_in_polygon(&polygons[i], valid_points.points[p]);
						int8_t index2 = get_point_index_in_polygon(&polygons[i], valid_points.points[p2]);
						if ((index == 0) && (index2 == (polygons[i].count - 1)))
							continue;
						if ((index2 == 0) && (index == (polygons[i].count - 1)))
							continue;
						if ((index >= 0) && (index2 >= 0) && (abs(index - index2) != 1))
						{
							collide++;
							break;
						}
					}
					if (collide != 0)
					{
						break;
					}
				}
				/* If no collision, both points of the segment are added to the graph with distance between them */
				if (collide == 0)
				{
					/* TODO, build the graph */
					if ((p < GRAPH_MAX_VERTICES) && (p2 < GRAPH_MAX_VERTICES))
					{
						graph[p][p2] = (valid_points.points[p2].x-valid_points.points[p].x);
						graph[p][p2] *= (valid_points.points[p2].x-valid_points.points[p].x);
						graph[p][p2] += (valid_points.points[p2].y-valid_points.points[p].y) * (valid_points.points[p2].y-valid_points.points[p].y);
						graph[p2][p] = graph[p][p2];
					}
				}
				else
				{
					if ((p < GRAPH_MAX_VERTICES) && (p2 < GRAPH_MAX_VERTICES))
					{
						graph[p][p2] = -1;
						graph[p2][p] = graph[p][p2];
					}
				}
			}
		}
	}

}

bool is_segment_crossing_line(pose_t a, pose_t b, pose_t o, pose_t p)
{
	vector_t ao, ap, ab;
	double det = 0;
	ab.x = b.x - a.x;
	ab.y = b.y - a.y;
	ap.x = p.x - a.x;
	ap.y = p.y - a.y;
	ao.x = o.x - a.x;
	ao.y = o.y - a.y;

	det = (ab.x * ap.y - ab.y * ap.x)*(ab.x * ao.y - ab.y * ao.x);

	if (det < 0)
		return true;
	else
		return false;
}

bool is_segment_crossing_segment(pose_t a,pose_t b,pose_t o,pose_t p)
{
	if (is_segment_crossing_line(a,b,o,p)==false)
		return false;
	if (is_segment_crossing_line(o,p,a,b)==false)
		return false;
	return true;
}

int8_t get_point_index_in_polygon(const polygon_t *polygons,pose_t p)
{
	uint8_t i;
	for(i = 0; i < polygons->count; i++)
	{
		if ((polygons->points[i].x == p.x) && (polygons->points[i].y == p.y))
		{
			return i;
		}
	}

	return -1;
}

bool is_point_in_polygon(const polygon_t *polygon, pose_t p)
{
	uint8_t i;
	double d;
	for(i = 0; i < polygon->count; i++)
	{
		pose_t a = polygon->points[i];
		pose_t b;
		b = (i == (polygon->count - 1) ? polygon->points[0] : polygon->points[i+1]);

		vector_t ab, ap;
		ab.x = b.x - a.x;
		ab.y = b.y - a.y;
		ap.x = p.x - a.x;
		ap.y = p.y - a.y;

		d = ab.x*ap.y - ab.y*ap.x;

		if (d < 0)
		{
			return false;
		}
	}

	return true;
}
