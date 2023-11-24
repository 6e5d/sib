#include <stdbool.h>
#include <stdint.h>

#include "../../simpleimg/include/simpleimg.h"
#include "../../dmgrect/include/dmgrect.h"

typedef struct {
	// config
	float spacing;
	float alpha_k;
	float alpha_b;
	float size_k;
	float size_b;
	uint8_t color[3]; // BGR

	// state
	float px;
	float py;
	float pp;
	bool drag;
} SibSimple;

void sib_simple_config(SibSimple *sib);
void sib_simple_update(SibSimple *sib, Simpleimg *img,
	Dmgrect *damage, float x, float y, float p);
void sib_simple_finish(SibSimple *sib);
