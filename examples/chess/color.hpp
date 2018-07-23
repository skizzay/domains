#pragma once

namespace chess {
enum class color : char { black, white };

enum class move_result_code : int { success, illegal_location, illegal_move, illegal_turn };
}
