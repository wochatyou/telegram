/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "data/data_peer.h"
#include "data/data_chat_participant_status.h"
#include "data/data_lastseen_status.h"
#include "data/data_user_names.h"
#include "dialogs/dialogs_key.h"

namespace Data {
struct BotCommand;
} // namespace Data

struct BotInfo {
	BotInfo();

	bool inited = false;
	bool readsAllHistory = false;
	bool cantJoinGroups = false;
	bool supportsAttachMenu = false;
	bool canEditInformation = false;
	int version = 0;
	int descriptionVersion = 0;
	QString description;
	QString inlinePlaceholder;
	std::vector<Data::BotCommand> commands;

	PhotoData *photo = nullptr;
	DocumentData *document = nullptr;

	QString botMenuButtonText;
	QString botMenuButtonUrl;

	QString startToken;
	Dialogs::EntryState inlineReturnTo;

	ChatAdminRights groupAdminRights;
	ChatAdminRights channelAdminRights;
};

enum class UserDataFlag : uint32 {
	Contact = (1 << 0),
	MutualContact = (1 << 1),
	Deleted = (1 << 2),
	Verified = (1 << 3),
	Scam = (1 << 4),
	Fake = (1 << 5),
	BotInlineGeo = (1 << 6),
	Blocked = (1 << 7),
	HasPhoneCalls = (1 << 8),
	PhoneCallsPrivate = (1 << 9),
	Support = (1 << 10),
	CanPinMessages = (1 << 11),
	DiscardMinPhoto = (1 << 12),
	Self = (1 << 13),
	Premium = (1 << 14),
	CanReceiveGifts = (1 << 15),
	VoiceMessagesForbidden = (1 << 16),
	PersonalPhoto = (1 << 17),
	StoriesHidden = (1 << 18),
	HasActiveStories = (1 << 19),
	HasUnreadStories = (1 << 20),
	MeRequiresPremiumToWrite = (1 << 21),
	SomeRequirePremiumToWrite = (1 << 22),
	RequirePremiumToWriteKnown = (1 << 23),
	ReadDatesPrivate = (1 << 24),
};
inline constexpr bool is_flag_type(UserDataFlag) { return true; };
using UserDataFlags = base::flags<UserDataFlag>;

[[nodiscard]] Data::LastseenStatus LastseenFromMTP(
	const MTPUserStatus &status,
	Data::LastseenStatus currentStatus);

class UserData final : public PeerData {
public:
	using Flag = UserDataFlag;
	using Flags = Data::Flags<UserDataFlags>;

	UserData(not_null<Data::Session*> owner, PeerId id);
	void setPhoto(const MTPUserProfilePhoto &photo);

	void setName(
		const QString &newFirstName,
		const QString &newLastName,
		const QString &newPhoneName,
		const QString &newUsername);
	void setUsernames(const Data::Usernames &newUsernames);

	void setUsername(const QString &username);
	void setPhone(const QString &newPhone);
	void setBotInfoVersion(int version);
	void setBotInfo(const MTPBotInfo &info);

	void setNameOrPhone(const QString &newNameOrPhone);

	void madeAction(TimeId when); // pseudo-online

	uint64 accessHash() const {
		return _accessHash;
	}
	void setAccessHash(uint64 accessHash);

	auto flags() const {
		return _flags.current();
	}
	auto flagsValue() const {
		return _flags.value();
	}
	void setFlags(UserDataFlags which);
	void addFlags(UserDataFlags which);
	void removeFlags(UserDataFlags which);

	[[nodiscard]] bool isVerified() const;
	[[nodiscard]] bool isScam() const;
	[[nodiscard]] bool isFake() const;
	[[nodiscard]] bool isPremium() const;
	[[nodiscard]] bool isBotInlineGeo() const;
	[[nodiscard]] bool isBot() const;
	[[nodiscard]] bool isSupport() const;
	[[nodiscard]] bool isInaccessible() const;
	[[nodiscard]] bool applyMinPhoto() const;
	[[nodiscard]] bool hasPersonalPhoto() const;
	[[nodiscard]] bool hasStoriesHidden() const;
	[[nodiscard]] bool someRequirePremiumToWrite() const;
	[[nodiscard]] bool meRequiresPremiumToWrite() const;
	[[nodiscard]] bool requirePremiumToWriteKnown() const;
	[[nodiscard]] bool canSendIgnoreRequirePremium() const;
	[[nodiscard]] bool readDatesPrivate() const;

	[[nodiscard]] bool canShareThisContact() const;
	[[nodiscard]] bool canAddContact() const;

	[[nodiscard]] bool canReceiveGifts() const;

	// In Data::Session::processUsers() we check only that.
	// When actually trying to share contact we perform
	// a full check by canShareThisContact() call.
	[[nodiscard]] bool canShareThisContactFast() const;

	MTPInputUser inputUser = MTP_inputUserEmpty();

	QString firstName;
	QString lastName;
	[[nodiscard]] const QString &phone() const;
	[[nodiscard]] QString username() const;
	[[nodiscard]] QString editableUsername() const;
	[[nodiscard]] const std::vector<QString> &usernames() const;
	QString nameOrPhone;

	enum class ContactStatus : char {
		Unknown,
		Contact,
		NotContact,
	};
	[[nodiscard]] ContactStatus contactStatus() const;
	[[nodiscard]] bool isContact() const;
	void setIsContact(bool is);

	[[nodiscard]] Data::LastseenStatus lastseen() const;
	bool updateLastseen(Data::LastseenStatus value);

	enum class CallsStatus : char {
		Unknown,
		Enabled,
		Disabled,
		Private,
	};
	CallsStatus callsStatus() const;
	bool hasCalls() const;
	void setCallsStatus(CallsStatus callsStatus);

	std::unique_ptr<BotInfo> botInfo;

	void setUnavailableReasons(
		std::vector<Data::UnavailableReason> &&reasons);

	int commonChatsCount() const;
	void setCommonChatsCount(int count);

	[[nodiscard]] bool hasPrivateForwardName() const;
	[[nodiscard]] QString privateForwardName() const;
	void setPrivateForwardName(const QString &name);

	[[nodiscard]] bool hasActiveStories() const;
	[[nodiscard]] bool hasUnreadStories() const;
	void setStoriesState(StoriesState state);

private:
	auto unavailableReasons() const
		-> const std::vector<Data::UnavailableReason> & override;

	Flags _flags;
	Data::LastseenStatus _lastseen;

	Data::UsernamesInfo _username;

	std::vector<Data::UnavailableReason> _unavailableReasons;
	QString _phone;
	QString _privateForwardName;
	ContactStatus _contactStatus = ContactStatus::Unknown;
	CallsStatus _callsStatus = CallsStatus::Unknown;
	int _commonChatsCount = 0;

	uint64 _accessHash = 0;
	static constexpr auto kInaccessibleAccessHashOld
		= 0xFFFFFFFFFFFFFFFFULL;

};

namespace Data {

void ApplyUserUpdate(not_null<UserData*> user, const MTPDuserFull &update);

} // namespace Data
