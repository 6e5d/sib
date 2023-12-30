#include "../../vwdlayer/build/vwdlayer.h"
#include "../../simpleimg/build/simpleimg.h"
#include "../../dmgrect/build/dmgrect.h"

typedef struct {
	Simpleimg() *canvas;
	// pending: 1 frame may have multiple segments
	// accum: 1 stroke may have many segments
	Dmgrect() pending;
	// config
	float spacing;
	float alpha_k;
	float alpha_b;
	float size_k;
	float size_b;
	float size_scale; // the primary setting is size
	uint8_t color[3]; // BGR
} Sib(Simple);

void sib(simple_config)(Sib(Simple) *sib);
void sib(simple_config_eraser)(Sib(Simple) *sib);
Vwdlayer(Ifdraw) sib(simple_ifdraw)(void);
