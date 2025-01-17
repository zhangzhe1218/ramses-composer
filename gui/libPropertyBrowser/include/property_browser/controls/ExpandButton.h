/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <QPushButton>
#include <QWidget>

namespace raco::property_browser {

class PropertyBrowserItem;

class ExpandControlButton final : public QPushButton {
	Q_OBJECT
public:
	explicit ExpandControlButton(PropertyBrowserItem* item, QWidget* parent = nullptr);


private Q_SLOTS:
	void updateIcon(bool expanded);
};

}  // namespace raco::property_browser
