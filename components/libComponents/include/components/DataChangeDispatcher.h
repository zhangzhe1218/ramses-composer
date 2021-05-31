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

#include "core/Context.h"

#include <functional>
#include <memory>
#include <vector>

namespace raco::components {

class BaseListener {
public:
	virtual ~BaseListener() {}
};
class DataChangeDispatcher;

class Subscription {
	friend class DataChangeDispatcher;
public:
	Subscription(const Subscription&) = delete;
	Subscription& operator=(const Subscription&) = delete;
	explicit Subscription() = default;
	/** Container only subscription which will destroy all subscriptions when it gets destroyed */
	explicit Subscription(std::vector<Subscription>&& subSubscriptions);
	
	using DeregisterCallback = std::function<void()>;
	explicit Subscription(DataChangeDispatcher* owner, const std::shared_ptr<BaseListener>& listener, DeregisterCallback callback);

	~Subscription();
	Subscription(Subscription&& other) {
		std::swap(other.dataChangeDispatcher_, dataChangeDispatcher_);
		std::swap(other.listener_, listener_);
		std::swap(other.subSubscriptions_, subSubscriptions_);
		std::swap(other.deregisterFunc_, deregisterFunc_);
	}
	Subscription& operator=(Subscription&& other) {
		std::swap(other.dataChangeDispatcher_, dataChangeDispatcher_);
		std::swap(other.listener_, listener_);
		std::swap(other.subSubscriptions_, subSubscriptions_);
		std::swap(other.deregisterFunc_, deregisterFunc_);
		return *this;
	}

private:
	DataChangeDispatcher* dataChangeDispatcher_ {nullptr};
	std::shared_ptr<BaseListener> listener_ {nullptr};
	std::vector<Subscription> subSubscriptions_ {};
	DeregisterCallback deregisterFunc_;
};

class LinkLifecycleListener;
class LinkListener;
class ObjectLifecycleListener;
class EditorObjectListener;
class PropertyChangeListener;
class ValueHandleListener;
class UndoListener;
class ChildrenListener;

class DataChangeDispatcher {
	DataChangeDispatcher(const DataChangeDispatcher&) = delete;
	DataChangeDispatcher& operator=(const DataChangeDispatcher&) = delete;

public:
	using Callback = std::function<void()>;
	using EditorObjectCallback = std::function<void(core::SEditorObject)>;
	using ValueHandleCallback = std::function<void(const core::ValueHandle&)>;
	using BulkChangeCallback = std::function<void(const std::set<core::SEditorObject>&)>;
	using LinkCallback = std::function<void(const core::LinkDescriptor&)>;

	explicit DataChangeDispatcher();

	Subscription registerOn(core::ValueHandle valueHandle, Callback callback) noexcept;
	Subscription registerOn(core::ValueHandles handles, ValueHandleCallback callback) noexcept;
	Subscription registerOnChildren(core::ValueHandle valueHandle, ValueHandleCallback callback) noexcept;
	Subscription registerOnPropertyChange(const std::string &propertyName, ValueHandleCallback callback) noexcept;
	Subscription registerOnObjectsLifeCycle(EditorObjectCallback onCreation, EditorObjectCallback onDeletion) noexcept;
	/** Lifecycle changes to any link, creation and deletion. */
	Subscription registerOnLinksLifeCycle(LinkCallback onCreation, LinkCallback onDeletion) noexcept;
	Subscription registerOnLinkValidityChange(LinkCallback callback) noexcept;
	Subscription registerOnErrorChanged(core::ValueHandle valueHandle, Callback callback) noexcept;
	Subscription registerOnPreviewDirty(core::SEditorObject obj, Callback callback) noexcept;
	Subscription registerOnUndoChanged(Callback callback) noexcept;

	Subscription registerOnExternalProjectChanged(Callback callback) noexcept;
	Subscription registerOnExternalProjectMapChanged(Callback callback) noexcept;

	// This will regisiter a callback which is invoked by dispatch() after all other changes have been dispatched.
	Subscription registerOnAfterDispatch(Callback callback);

	void registerBulkChangeCallback(BulkChangeCallback callback);
	void resetBulkChangeCallback();

	void dispatch(const core::DataChangeRecorder &dataChanges);

	void assertEmpty();

	void setUndoChanged() {
		undoChanged_ = true;
	}

	void setExternalProjectChanged() {
		externalProjectChanged_ = true;
	}

private:
	void emitUpdateFor(const core::ValueHandle& valueHandle);
	void emitErrorChanged(const core::ValueHandle& valueHandle);
	void emitCreated(core::SEditorObject obj);
	void emitDeleted(core::SEditorObject obj);
	void emitPreviewDirty(core::SEditorObject obj);
	void emitBulkChange(const std::set<core::SEditorObject>& changedObjects);

	std::set<std::weak_ptr<ObjectLifecycleListener>, std::owner_less<std::weak_ptr<ObjectLifecycleListener>>> objectLifecycleListeners_{};
	std::set<std::weak_ptr<LinkLifecycleListener>, std::owner_less<std::weak_ptr<LinkLifecycleListener>>> linkLifecycleListeners_{};
	std::set<std::weak_ptr<LinkListener>, std::owner_less<std::weak_ptr<LinkListener>>> linkValidityChangeListeners_{};
	std::set<std::weak_ptr<ValueHandleListener>, std::owner_less<std::weak_ptr<ValueHandleListener>>> listeners_{};
	std::set<std::weak_ptr<ChildrenListener>, std::owner_less<std::weak_ptr<ChildrenListener>>> childrenListeners_{};
	std::set<std::weak_ptr<EditorObjectListener>, std::owner_less<std::weak_ptr<EditorObjectListener>>> objectChangeListeners_{};
	std::set<std::weak_ptr<EditorObjectListener>, std::owner_less<std::weak_ptr<EditorObjectListener>>> previewDirtyListeners_{};
	std::set<std::weak_ptr<ValueHandleListener>, std::owner_less<std::weak_ptr<ValueHandleListener>>> errorChangedListeners_{};
	std::set<std::weak_ptr<PropertyChangeListener>, std::owner_less<std::weak_ptr<PropertyChangeListener>>> propertyChangeListeners_{};

	bool undoChanged_ { false };
	std::set<std::weak_ptr<UndoListener>, std::owner_less<std::weak_ptr<UndoListener>>> undoChangeListeners_{};

	bool externalProjectChanged_{false};
	std::set<std::weak_ptr<UndoListener>, std::owner_less<std::weak_ptr<UndoListener>>> externalProjectChangedListeners_{};
	std::set<std::weak_ptr<UndoListener>, std::owner_less<std::weak_ptr<UndoListener>>> externalProjectMapChangedListeners_{};


	std::set<std::weak_ptr<UndoListener>, std::owner_less<std::weak_ptr<UndoListener>>> onAfterDispatchListeners_{};

	BulkChangeCallback bulkChangeCallback_;
};

using SDataChangeDispatcher = std::shared_ptr<DataChangeDispatcher>;

}  // namespace raco::core