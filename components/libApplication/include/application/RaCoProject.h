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

#include "components/MeshCacheImpl.h"
#include "components/Naming.h"
#include "core/CommandInterface.h"
#include "core/Context.h"
#include "core/Errors.h"
#include "core/Project.h"
#include "core/Undo.h"
#include "core/Serialization.h"
#include "user_types/UserObjectFactory.h"
#include <QObject>
#include <exception>
#include <functional>

namespace raco::application {

class RaCoApplication;

struct FutureFileVersion : public std::exception {
	explicit FutureFileVersion(int fileVersion) : fileVersion_{fileVersion} {}
	int fileVersion_;
	const char* what() const throw() {
		return "FutureFileVersion";
	}
};

class RaCoProject : public QObject {
	Q_OBJECT
public:
	Q_DISABLE_COPY(RaCoProject);
	~RaCoProject();

	static std::unique_ptr<RaCoProject> createNew(RaCoApplication* app);
	/**
	 * @exception FutureFileVersion when the loaded file contains a file version which is bigger than the known versions
	 * @exception ExtrefError
	 */
	static std::unique_ptr<RaCoProject> loadFromFile(const QString& filename, RaCoApplication* app, std::vector<std::string>& pathStack);
	static std::unique_ptr<RaCoProject> loadFromJson(const QJsonDocument& migratedJson, const QString& filename, RaCoApplication* app, std::vector<std::string>& pathStack);

	QString name() const;

	bool dirty() const noexcept;
	bool save();
	bool saveAs(const QString& fileName, bool setProjectName = false);

	// @exception ExtrefError
	void updateExternalReferences(std::vector<std::string>& pathStack);

	raco::core::Project* project();
	raco::core::Errors* errors();
	raco::core::DataChangeRecorder* recorder();
	raco::core::CommandInterface* commandInterface();
	raco::core::UndoStack* undoStack();
	raco::core::MeshCache* meshCache();

	QJsonDocument serializeProject(const std::unordered_map<std::string, std::vector<int>>& currentVersions);

Q_SIGNALS:
	void activeProjectFileChanged();

private:
	// @exception ExtrefError
	RaCoProject(const QString& file, raco::core::Project& p, raco::core::EngineInterface* engineInterface, const raco::core::UndoStack::Callback& callback, raco::core::ExternalProjectsStoreInterface* externalProjectsStore, RaCoApplication* app, std::vector<std::string>& pathStack);

	void onAfterProjectPathChange(const std::string& oldPath, const std::string& newPath);
	void generateProjectSubfolder(const std::string& subFolderPath);
	void generateAllProjectSubfolders();
	void updateActiveFileListener();

	raco::core::DataChangeRecorder recorder_;
	raco::core::Errors errors_;
	raco::core::Project project_;

	std::shared_ptr<raco::core::BaseContext> context_;
	bool dirty_{false};

	components::ProjectFileChangeMonitor activeProjectFileChangeMonitor_;
	raco::components::ProjectFileChangeMonitor::UniqueListener activeProjectFileChangeListener_;

	raco::core::MeshCache* meshCache_;
	raco::core::UndoStack undoStack_;
	raco::core::CommandInterface commandInterface_;
};

}  // namespace raco::application
