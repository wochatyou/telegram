/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "boxes/report_messages_box.h"

#include "api/api_report.h"
#include "data/data_peer.h"
#include "data/data_photo.h"
#include "lang/lang_keys.h"
#include "ui/boxes/report_box.h"
#include "ui/layers/generic_box.h"
#include "window/window_session_controller.h"
#include "styles/style_chat_helpers.h"

namespace {

[[nodiscard]] object_ptr<Ui::BoxContent> Report(
		not_null<PeerData*> peer,
		std::variant<
			v::null_t,
			MessageIdsList,
			not_null<PhotoData*>,
			StoryId> data,
		const style::ReportBox *stOverride) {
	const auto source = v::match(data, [](const MessageIdsList &ids) {
		return Ui::ReportSource::Message;
	}, [&](not_null<PhotoData*> photo) {
		return peer->isUser()
			? (photo->hasVideo()
				? Ui::ReportSource::ProfileVideo
				: Ui::ReportSource::ProfilePhoto)
			: (peer->isChat() || (peer->isChannel() && peer->isMegagroup()))
			? (photo->hasVideo()
				? Ui::ReportSource::GroupVideo
				: Ui::ReportSource::GroupPhoto)
			: (photo->hasVideo()
				? Ui::ReportSource::ChannelVideo
				: Ui::ReportSource::ChannelPhoto);
	}, [&](StoryId id) {
		return Ui::ReportSource::Story;
	}, [](v::null_t) {
		Unexpected("Bad source report.");
		return Ui::ReportSource::Bot;
	});
	const auto st = stOverride ? stOverride : &st::defaultReportBox;
	return Box([=](not_null<Ui::GenericBox*> box) {
		const auto show = box->uiShow();
		Ui::ReportReasonBox(box, *st, source, [=](Ui::ReportReason reason) {
			show->showBox(Box([=](not_null<Ui::GenericBox*> box) {
				Ui::ReportDetailsBox(box, *st, [=](const QString &text) {
					Api::SendReport(show, peer, reason, text, data);
					show->hideLayer();
				});
			}));
		});
	});
}

} // namespace

object_ptr<Ui::BoxContent> ReportItemsBox(
		not_null<PeerData*> peer,
		MessageIdsList ids) {
	return Report(peer, ids, nullptr);
}

object_ptr<Ui::BoxContent> ReportProfilePhotoBox(
		not_null<PeerData*> peer,
		not_null<PhotoData*> photo) {
	return Report(peer, photo, nullptr);
}

void ShowReportPeerBox(
		not_null<Window::SessionController*> window,
		not_null<PeerData*> peer) {
	struct State {
		QPointer<Ui::BoxContent> reasonBox;
		QPointer<Ui::BoxContent> detailsBox;
		MessageIdsList ids;
	};
	const auto state = std::make_shared<State>();
	const auto chosen = [=](Ui::ReportReason reason) {
		const auto send = [=](const QString &text) {
			window->clearChooseReportMessages();
			Api::SendReport(
				window->uiShow(),
				peer,
				reason,
				text,
				std::move(state->ids));
			if (const auto strong = state->reasonBox.data()) {
				strong->closeBox();
			}
			if (const auto strong = state->detailsBox.data()) {
				strong->closeBox();
			}
		};
		if (reason == Ui::ReportReason::Fake
			|| reason == Ui::ReportReason::Other) {
			state->ids = {};
			state->detailsBox = window->show(
				Box(Ui::ReportDetailsBox, st::defaultReportBox, send));
			return;
		}
		window->showChooseReportMessages(peer, reason, [=](
				MessageIdsList ids) {
			state->ids = std::move(ids);
			state->detailsBox = window->show(
				Box(Ui::ReportDetailsBox, st::defaultReportBox, send));
		});
	};
	state->reasonBox = window->show(Box(
		Ui::ReportReasonBox,
		st::defaultReportBox,
		(peer->isBroadcast()
			? Ui::ReportSource::Channel
			: peer->isUser()
			? Ui::ReportSource::Bot
			: Ui::ReportSource::Group),
		chosen));
}
