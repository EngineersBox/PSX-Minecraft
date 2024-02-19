#pragma once

#ifndef PSX_MINECRAFT_AXIS_H
#define PSX_MINECRAFT_AXIS_H

#include "../render/render_context.h"
#include "../render/transforms.h"
#include "../core/camera.h"

void axisDraw(RenderContext* ctx, const Transforms* transforms, const Camera* camera);

#endif // PSX_MINECRAFT_AXIS_H
