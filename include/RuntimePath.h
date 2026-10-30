#ifndef RUNTIMEPATH_H
#define RUNTIMEPATH_H

#include "Handles.h"

#include <vector>

/**
 * @brief Identifies one runtime instance without borrowing its storage.
 *
 * Root IDs are scoped to a controller and never reused. Each placement ID is
 * scoped to the preceding instance's blueprint. Removing a placement or stopping
 * the root makes the path invalid; it never selects a replacement instance.
 */
struct RuntimePath
{
    RuntimePath(RuntimeId root = RuntimeId()) : root(root) {}

    RuntimeId root;
    std::vector< ComponentId > placements;
};

#endif // RUNTIMEPATH_H
