/* Icinga 2 | (c) 2012 Icinga GmbH | GPLv2+ */

#ifndef NOTIFICATIONCOMPONENT_H
#define NOTIFICATIONCOMPONENT_H

#include "notification/notificationcomponent-ti.hpp"
#include "icinga/service.hpp"
#include "base/configobject.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <thread>

namespace icinga
{

/**
 * @ingroup notification
 */
	struct NotificationScheduleInfo
	{
		Notification::Ptr Object;
		double NextMessage;
	};

/**
 * @ingroup notification
 */
	struct NotificationNextMessageExtractor
	{
		typedef double result_type;

		/**
		 * @threadsafety Always.
		 */
		double operator()(const NotificationScheduleInfo& nsi)
		{
			return nsi.NextMessage;
		}
	};

/**
 * @ingroup notification
 */
class NotificationComponent final : public ObjectImpl<NotificationComponent>
{
public:
	DECLARE_OBJECT(NotificationComponent);
	DECLARE_OBJECTNAME(NotificationComponent);

	typedef boost::multi_index_container<
			NotificationScheduleInfo,
			boost::multi_index::indexed_by<
					boost::multi_index::ordered_unique<boost::multi_index::member<NotificationScheduleInfo, Notification::Ptr, &NotificationScheduleInfo::Object> >,
					boost::multi_index::ordered_non_unique<NotificationNextMessageExtractor>
			>
	> NotificationSet;

	void Start(bool runtimeCreated) override;
	void Stop(bool runtimeRemoved) override;

	static void StatsFunc(const Dictionary::Ptr& status, const Array::Ptr& perfdata);
	unsigned long GetIdleNotifications();
	unsigned long GetPendingNotifications();

private:
	boost::mutex m_Mutex;
	boost::condition_variable m_CV;
	bool m_Stopped{false};
	std::thread m_Thread;

	NotificationSet m_IdleNotifications;
	NotificationSet m_PendingNotifications;

	void StateChangeHandler(const Checkable::Ptr& checkable, const CheckResult::Ptr& cr, StateType type);
	void FlappingChangedHandler(const Checkable::Ptr& checkable);

	void NextNotificationChangedHandler(const Notification::Ptr& notification, const MessageOrigin::Ptr& origin);

	void NotificationThreadProc();
	void SendMessageHelper(const Notification::Ptr& notification, NotificationType type, bool reminder);
	NotificationScheduleInfo GetNotificationScheduleInfo(const Notification::Ptr& notification);

	bool HardStateNotificationCheck(const Checkable::Ptr& checkable);
};

}

#endif /* NOTIFICATIONCOMPONENT_H */
