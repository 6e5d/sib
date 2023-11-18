#include <stdbool.h>

#include "../../simpleimg/include/simpleimg.h"

typedef struct {
	// config
	float spacing;
	float alpha_k;
	float alpha_b;
	float size_k;
	float size_b;

	// state
	float px;
	float py;
	float pp;
	bool drag;
} SibSimple;

void sib_simple_config(SibSimple *sib);
void sib_simple_update(SibSimple *sib, Simpleimg *img,
	float x, float y, float p);
void sib_simple_finish(SibSimple *sib);
