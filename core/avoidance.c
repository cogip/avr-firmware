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

/* List of visible points */
static pose_t valid_points[MAX_POINTS];
static uint8_t valid_points_count = 0;

static uint64_t graph[GRAPH_MAX_VERTICES];

pose_t avoidance(const pose_t *start, const pose_t *finish)
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
			return *start;
		}
	}

	valid_points_count = 0;
	valid_points[valid_points_count++] = *start;
	valid_points[valid_points_count++] = *finish;

	/* Build path graph */
	build_avoidance_graph();
	return dijkstra(1);
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

	nb_polygons = 0;

	polygon.count = 0;
	nb_vertices = 4;
	if (nb_vertices < POLY_MAX_POINTS)
	{
		polygon.points[polygon.count++] = (pose_t){.x = 1000, .y = 600};
		polygon.points[polygon.count++] = (pose_t){.x = 1400, .y = 600};
		polygon.points[polygon.count++] = (pose_t){.x = 1400, .y = 1000};
		polygon.points[polygon.count++] = (pose_t){.x = 1000, .y = 1000};
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
				valid_points[valid_points_count++] = polygons[i].points[p];
			}
		}
	}

	/* For each segment of the valid points list */
	for (int p = 0; p < valid_points_count; p++)
	{
		for (int p2 = p+1; p2 < valid_points_count; p2++)
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

						if (is_segment_crossing_segment(valid_points[p],valid_points[p2], polygons[i].points[v], p_next) == true)
						{
							collide++;
							break;
						}
						/* Special case of internal crossing of a polygon */
						int8_t index = get_point_index_in_polygon(&polygons[i], valid_points[p]);
						int8_t index2 = get_point_index_in_polygon(&polygons[i], valid_points[p2]);
						if ((index == 0) && (index2 == (polygons[i].count - 1)))
							continue;
						if ((index2 == 0) && (index == (polygons[i].count - 1)))
							continue;
						if ((index >= 0) && (index2 >= 0) && (abs(index - index2) != 1))
						{
							collide++;
							break;
						}
						if (is_point_on_segment(valid_points[p],valid_points[p2], polygons[i].points[v]) == true)
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
				if ((p < GRAPH_MAX_VERTICES) && (p2 < GRAPH_MAX_VERTICES))
				{
					if (collide == 0)
					{
						graph[p] |= (1 << p2);
						graph[p2] |= (1 << p);
					}
					else
					{
						graph[p] &= ~(1 << p2);
						graph[p2] &= ~(1 << p);
					}
				}
			}
		}
	}
}

bool is_point_on_segment(pose_t a, pose_t b, pose_t o)
{
	bool res = false;

	if ((b.x - a.x) / (b.y - a.y) == (b.x - o.x) / (b.y - o.y))
	{
		if (a.x < b.x)
		{
			if ((o.x < b.x) && (o.x > a.x))
			{
				res = true;
			}
		}
		else
		{
			if ((o.x < a.x) && (o.x > b.x))
			{
				res = true;
			}
		}
	}

	return res;
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

		if (d <= 0)
		{
			return false;
		}
	}

	return true;
}


pose_t dijkstra(uint16_t target)
{
	bool checked[GRAPH_MAX_VERTICES];
	double distance[GRAPH_MAX_VERTICES];
	uint16_t v;
	int i;
	double weight;
	double min_distance;
	int parent[GRAPH_MAX_VERTICES];
	/* TODO: start should be a parameter. More clean even if start is always index 0 in our case */
	int start = 0;

	for (int i = 0; i <= valid_points_count; i++)
	{
		checked[i] = false;
		distance[i] = DIJKSTRA_MAX_DISTANCE;
		parent[i] = -1;
	}

	distance[start] = 0;
	v = start;

	while ((v != target) && (checked[v] == false))
	{
		min_distance = DIJKSTRA_MAX_DISTANCE;
		checked[v] = true;
		for (i = 0; i < valid_points_count; i++)
		{
			if (graph[v] & (1 << i))
			{
				weight = (valid_points[v].x - valid_points[i].x);
				weight *= (valid_points[v].x - valid_points[i].x);
				weight += (valid_points[v].y - valid_points[i].y)
					* (valid_points[v].y - valid_points[i].y);
				weight = sqrt(weight);
				if ((weight >= 0 ) && (distance[i] > (distance[v] + weight)))
				{
					distance[i] = distance[v] + weight;
					parent[i] = v;
				}
			}
		}
		for (i = 1; i < valid_points_count; i++)
		{
			if ((checked[i] == false) && (min_distance > distance[i]))
			{
				min_distance = distance[i];
				v = i;
			}
		}
	}

	i = 1;
	while (parent[i] > 0)
		i = parent[i];

	return valid_points[i];
}
