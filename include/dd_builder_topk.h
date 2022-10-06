#pragma once

#include "encoder_abstract.h"
#include "options.h"
#include "dd_buildable.h"

namespace dd_builder{
    void construct_dd_top_k(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
}