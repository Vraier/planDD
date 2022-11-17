#pragma once

#include "dd_buildable.h"
#include "encoder_abstract.h"
#include "options.h"

namespace dd_builder {
void construct_dd_top_k(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
void construct_dd_top_k_with_all_goals(dd_buildable &container, encoder::encoder_abstract &encoder,
                                       option_values &options);
}  // namespace dd_builder