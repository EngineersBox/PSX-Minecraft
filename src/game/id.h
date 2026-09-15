#pragma once

#ifndef _GAME__ID_H_
#define _GAME__ID_H_

#define CompositeID(id_type, metadata_id_type, joined_type) union { \
    struct { \
        id_type id; \
        metadata_id_type metadata_id; \
    }; \
    joined_type joined_id; \
}

#endif // _GAME__ID_H_
