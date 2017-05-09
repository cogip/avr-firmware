#include "odometry.h"

#define POLY_MAX 64
#define POLY_MAX_POINTS 128

/* Boolean */
/* TODO: define this elsewhere */
typedef enum
{
	true = 0,
	false
} bool;

/* Vector */
/* TODO: should it be generic to all core functions ? */
typedef struct
{
	double x,y;
} vector_t;

/* Polygon */
/* TODO: should it be generic to all core functions ? */
typedef struct
{
	uint8_t count;
	pose_t *points;
} polygon_t;

void init_polygons(void);
void build_avoidance_graph(void);
int add_polygon(polygon_t *polygon);
bool is_point_in_polygon(const polygon_t *polygons,pose_t p);
int8_t get_point_index_in_polygon(const polygon_t *polygons,pose_t p);
bool is_segment_crossing_line(pose_t a, pose_t b, pose_t o, pose_t p);
bool is_segment_crossing_segment(pose_t a,pose_t b,pose_t o,pose_t p);
