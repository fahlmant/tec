#pragma once

#include <glm/mat4x4.hpp>

#include "../proto/components.pb.h"

namespace tec {
	struct View {
		glm::mat4 view_matrix;
		bool active = false;

		void Out(proto::Component* target) {
			proto::View* comp = target->mutable_view();
			comp->set_active(this->active);
		}

		void In(const proto::Component& source) {
			const proto::View& comp = source.view();
			if (comp.has_active()) {
				this->active = comp.active();
			}
		}
	};
}