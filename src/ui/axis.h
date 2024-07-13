#pragma once

#ifndef PSXMC_AXIS_H
#define PSXMC_AXIS_H

#include "../render/render_context.h"
#include "../render/transforms.h"
#include "../core/camera.h"

void axisDraw(RenderContext* ctx, const Transforms* transforms, const Camera* camera);

#endif // PSXMC_AXIS_H
