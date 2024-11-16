/**************************************************************************/
/*  steam_ugc.cpp                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                           EIRTeam.Steamworks                           */
/*                         https://ph.eirteam.moe                         */
/**************************************************************************/
/* Copyright (c) 2023-present Álex Román (EIRTeam) & contributors.        */
/*                                                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "steam_ugc.h"
#include "steam/steam_api_flat.h"
#include "steamworks.h"

void HBSteamUGCQuery::_apply_returns(QueryScopeType p_query_type) {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	if (wants_only_ids && p_query_type == QUERY_DETAILS) {
		ERR_PRINT_ONCE_ED("Steamworks: with_only_ids cannot be used when querying for specific files");
	} else if (wants_only_ids) {
		SteamAPI_ISteamUGC_SetReturnOnlyIDs(iugc, query_handle, wants_only_ids);
	}
	SteamAPI_ISteamUGC_SetReturnKeyValueTags(iugc, query_handle, wants_key_value_tags);
	SteamAPI_ISteamUGC_SetReturnLongDescription(iugc, query_handle, wants_long_description);
	SteamAPI_ISteamUGC_SetReturnMetadata(iugc, query_handle, wants_metadata);
	SteamAPI_ISteamUGC_SetReturnChildren(iugc, query_handle, wants_children);
	SteamAPI_ISteamUGC_SetReturnAdditionalPreviews(iugc, query_handle, wants_additional_previews);

	if (wants_total_only && p_query_type == QUERY_DETAILS) {
		ERR_PRINT_ONCE_ED("Steamworks: with_total_only cannot be used when querying for specific files");
	} else if (wants_total_only) {
		SteamAPI_ISteamUGC_SetReturnTotalOnly(iugc, query_handle, wants_total_only);
	}
	SteamAPI_ISteamUGC_SetReturnPlaytimeStats(iugc, query_handle, wants_playtime_stats);
}

void HBSteamUGCQuery::_apply_constraints(QueryScopeType p_query_type) {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	if (required_tags.size() > 0) {
		for (String &tag : required_tags) {
			SteamAPI_ISteamUGC_AddRequiredTag(iugc, query_handle, tag.utf8().get_data());
		}
	}
	if (excluded_tags.size() > 0) {
		for (String &tag : excluded_tags) {
			SteamAPI_ISteamUGC_AddExcludedTag(iugc, query_handle, tag.utf8().get_data());
		}
	}
	if (required_kv.size() > 0) {
		for (const KeyValue<String, String> &tag : required_kv) {
			SteamAPI_ISteamUGC_AddRequiredKeyValueTag(iugc, query_handle, tag.key.utf8().get_data(), tag.value.utf8().get_data());
		}
	}

	if (p_query_type != QUERY_ALL && has_match_any_tag) {
		ERR_PRINT_ONCE_ED("Steamworks: match_any_tag can only be used when queryning for all UGC files.");
	} else if (has_match_any_tag) {
		SteamAPI_ISteamUGC_SetMatchAnyTag(iugc, query_handle, match_any_tag_value);
	}

	if (trend_days > 0) {
		SteamAPI_ISteamUGC_SetRankedByTrendDays(iugc, query_handle, trend_days);
	}

	if (!search_text.is_empty()) {
		SteamAPI_ISteamUGC_SetSearchText(iugc, query_handle, search_text.utf8().get_data());
	}
}

void HBSteamUGCQuery::_on_query_completed(Ref<SteamworksCallbackData> p_callback, bool p_io_failure, int p_page) {
	const SteamUGCQueryCompleted_t *query_completed = p_callback->get_data<SteamUGCQueryCompleted_t>();
	DEV_ASSERT(page_infos.has(query_completed->m_handle));
	HBSteamUGCQueryPageResult::ResultPageInfo page_info = page_infos[query_completed->m_handle];
	page_infos.erase(query_completed->m_handle);
	page_info.data_cached = query_completed->m_bCachedData;
	page_info.total_results = query_completed->m_unTotalMatchingResults;
	page_info.result_count = query_completed->m_unNumResultsReturned;

	Ref<HBSteamUGCQueryPageResult> page_result = memnew(HBSteamUGCQueryPageResult(page_info));
	emit_signal("query_completed", page_result);
}

void HBSteamUGCQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ranked_by_vote"), &HBSteamUGCQuery::ranked_by_vote);
	ClassDB::bind_method(D_METHOD("ranked_by_publication_date"), &HBSteamUGCQuery::ranked_by_publication_date);
	ClassDB::bind_method(D_METHOD("ranked_by_acceptance_date"), &HBSteamUGCQuery::ranked_by_acceptance_date);
	ClassDB::bind_method(D_METHOD("ranked_by_trend"), &HBSteamUGCQuery::ranked_by_trend);
	ClassDB::bind_method(D_METHOD("favorited_by_friends"), &HBSteamUGCQuery::favorited_by_friends);
	ClassDB::bind_method(D_METHOD("created_by_friends"), &HBSteamUGCQuery::created_by_friends);
	ClassDB::bind_method(D_METHOD("ranked_by_num_times_reported"), &HBSteamUGCQuery::ranked_by_num_times_reported);
	ClassDB::bind_method(D_METHOD("created_by_followed_users"), &HBSteamUGCQuery::created_by_followed_users);
	ClassDB::bind_method(D_METHOD("not_yet_rated"), &HBSteamUGCQuery::not_yet_rated);
	ClassDB::bind_method(D_METHOD("ranked_by_total_votes_asc"), &HBSteamUGCQuery::ranked_by_total_votes_asc);
	ClassDB::bind_method(D_METHOD("ranked_by_text_search"), &HBSteamUGCQuery::ranked_by_text_search);
	ClassDB::bind_method(D_METHOD("ranked_by_total_unique_subscriptions"), &HBSteamUGCQuery::ranked_by_total_unique_subscriptions);
	ClassDB::bind_method(D_METHOD("ranked_by_playtime_trend"), &HBSteamUGCQuery::ranked_by_playtime_trend);
	ClassDB::bind_method(D_METHOD("ranked_by_total_playtime"), &HBSteamUGCQuery::ranked_by_total_playtime);
	ClassDB::bind_method(D_METHOD("ranked_by_average_playtime_trend"), &HBSteamUGCQuery::ranked_by_average_playtime_trend);
	ClassDB::bind_method(D_METHOD("ranked_by_lifetime_average_playtime"), &HBSteamUGCQuery::ranked_by_lifetime_average_playtime);
	ClassDB::bind_method(D_METHOD("ranked_by_playtime_sessions_trend"), &HBSteamUGCQuery::ranked_by_playtime_sessions_trend);
	ClassDB::bind_method(D_METHOD("ranked_by_lifetime_playtime_sessions"), &HBSteamUGCQuery::ranked_by_lifetime_playtime_sessions);

	ClassDB::bind_method(D_METHOD("with_user", "user"), &HBSteamUGCQuery::with_user);
	ClassDB::bind_method(D_METHOD("where_user_published"), &HBSteamUGCQuery::where_user_published);
	ClassDB::bind_method(D_METHOD("where_user_voted_on"), &HBSteamUGCQuery::where_user_voted_on);
	ClassDB::bind_method(D_METHOD("where_user_voted_up"), &HBSteamUGCQuery::where_user_voted_up);
	ClassDB::bind_method(D_METHOD("where_user_voted_down"), &HBSteamUGCQuery::where_user_voted_down);
	ClassDB::bind_method(D_METHOD("where_user_will_vote_later"), &HBSteamUGCQuery::where_user_will_vote_later);
	ClassDB::bind_method(D_METHOD("where_user_favorited"), &HBSteamUGCQuery::where_user_favorited);
	ClassDB::bind_method(D_METHOD("where_user_subscribed"), &HBSteamUGCQuery::where_user_subscribed);
	ClassDB::bind_method(D_METHOD("where_user_used_or_played"), &HBSteamUGCQuery::where_user_used_or_played);
	ClassDB::bind_method(D_METHOD("where_user_followed"), &HBSteamUGCQuery::where_user_followed);

	ClassDB::bind_method(D_METHOD("sort_by_creation_date"), &HBSteamUGCQuery::sort_by_creation_date);
	ClassDB::bind_method(D_METHOD("sort_by_creation_date_asc"), &HBSteamUGCQuery::sort_by_creation_date_asc);
	ClassDB::bind_method(D_METHOD("sort_by_title_asc"), &HBSteamUGCQuery::sort_by_title_asc);
	ClassDB::bind_method(D_METHOD("sort_by_update_date"), &HBSteamUGCQuery::sort_by_update_date);
	ClassDB::bind_method(D_METHOD("sort_by_subscription_date"), &HBSteamUGCQuery::sort_by_subscription_date);
	ClassDB::bind_method(D_METHOD("sort_by_vote_score"), &HBSteamUGCQuery::sort_by_vote_score);
	ClassDB::bind_method(D_METHOD("sort_by_moderation"), &HBSteamUGCQuery::sort_by_moderation);

	ClassDB::bind_method(D_METHOD("where_search_text", "search_text"), &HBSteamUGCQuery::where_search_text);
	ClassDB::bind_method(D_METHOD("with_file_ids", "file_ids"), &HBSteamUGCQuery::with_file_ids);
	ClassDB::bind_method(D_METHOD("with_type", "matching_type"), &HBSteamUGCQuery::with_type);
	ClassDB::bind_method(D_METHOD("allow_cached_response", "max_age_seconds"), &HBSteamUGCQuery::allow_cached_response);
	ClassDB::bind_method(D_METHOD("in_language", "language"), &HBSteamUGCQuery::in_language);
	ClassDB::bind_method(D_METHOD("with_trend_days", "trend_days"), &HBSteamUGCQuery::with_trend_days);

	ClassDB::bind_method(D_METHOD("match_any_tag"), &HBSteamUGCQuery::match_any_tag);
	ClassDB::bind_method(D_METHOD("match_all_tags"), &HBSteamUGCQuery::match_all_tags);
	ClassDB::bind_method(D_METHOD("with_tag", "tag"), &HBSteamUGCQuery::with_tag);
	ClassDB::bind_method(D_METHOD("add_required_key_value_tag", "key", "value"), &HBSteamUGCQuery::add_required_key_value_tag);
	ClassDB::bind_method(D_METHOD("without_tag", "tag"), &HBSteamUGCQuery::without_tag);

	ClassDB::bind_method(D_METHOD("with_only_ids", "with_only_ids"), &HBSteamUGCQuery::with_only_ids);
	ClassDB::bind_method(D_METHOD("with_key_value_tags", "with_key_value_tags"), &HBSteamUGCQuery::with_key_value_tags);
	ClassDB::bind_method(D_METHOD("with_long_description", "with_long_description"), &HBSteamUGCQuery::with_long_description);
	ClassDB::bind_method(D_METHOD("with_metadata", "with_metadata"), &HBSteamUGCQuery::with_metadata);
	ClassDB::bind_method(D_METHOD("with_children", "with_children"), &HBSteamUGCQuery::with_children);
	ClassDB::bind_method(D_METHOD("with_additional_previews", "with_additional_previews"), &HBSteamUGCQuery::with_additional_previews);
	ClassDB::bind_method(D_METHOD("with_total_only", "with_total_only"), &HBSteamUGCQuery::with_total_only);
	ClassDB::bind_method(D_METHOD("with_playtime_stats", "with_playtime_stats"), &HBSteamUGCQuery::with_playtime_stats);

	ClassDB::bind_method(D_METHOD("request_page", "page"), &HBSteamUGCQuery::request_page);

	ClassDB::bind_static_method("HBSteamUGCQuery", D_METHOD("create_query", "matching_type"), &HBSteamUGCQuery::create_query);

	ADD_SIGNAL(MethodInfo("query_completed", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamUGCQueryPageResult")));
}

HBSteamUGCQuery::HBSteamUGCQuery(SWC::UGCMatchingUGCType p_matching_type) {
	matching_type = p_matching_type;
}

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_vote() {
	query_type = SWC::UGC_QUERY_RANKED_BY_VOTE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_publication_date() {
	query_type = SWC::UGC_QUERY_RANKED_BY_PUBLICATION_DATE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_acceptance_date() {
	query_type = SWC::UGC_QUERY_ACCEPTED_FOR_GAME_RANKED_BY_ACCEPTANCE_DATE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_trend() {
	query_type = SWC::UGC_QUERY_RANKED_BY_TREND;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::favorited_by_friends() {
	query_type = SWC::UGC_QUERY_FAVORITED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::created_by_friends() {
	query_type = SWC::UGC_QUERY_CREATED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_num_times_reported() {
	query_type = SWC::UGC_QUERY_RANKED_BY_NUM_TIMES_REPORTED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::created_by_followed_users() {
	query_type = SWC::UGC_QUERY_CREATED_BY_FOLLOWED_USERS_RANKED_BY_PUBLICATION_DATE;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::not_yet_rated() {
	query_type = SWC::UGC_QUERY_NOT_YET_RATED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_total_votes_asc() {
	query_type = SWC::UGC_QUERY_RANKED_BY_TOTAL_VOTES_ASC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_text_search() {
	query_type = SWC::UGC_QUERY_RANKED_BY_TEXT_SEARCH;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_total_unique_subscriptions() {
	query_type = SWC::UGC_QUERY_RANKED_BY_TOTAL_UNIQUE_SUBSCRIPTIONS;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_playtime_trend() {
	query_type = SWC::UGC_QUERY_RANKED_BY_PLAYTIME_TREND;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_total_playtime() {
	query_type = SWC::UGC_QUERY_RANKED_BY_TOTAL_PLAYTIME;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_average_playtime_trend() {
	query_type = SWC::UGC_QUERY_RANKED_BY_AVERAGE_PLAYTIME_TREND;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_lifetime_average_playtime() {
	query_type = SWC::UGC_QUERY_RANKED_BY_LIFETIME_AVERAGE_PLAYTIME;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_playtime_sessions_trend() {
	query_type = SWC::UGC_QUERY_RANKED_BY_PLAYTIME_SESSIONS_TREND;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::ranked_by_lifetime_playtime_sessions() {
	query_type = SWC::UGC_QUERY_RANKED_BY_LIFETIME_PLAYTIME_SESSIONS;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_published() {
	user_type = SWC::USER_UGC_LIST_PUBLISHED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_voted_on() {
	user_type = SWC::USER_UGC_LIST_VOTED_ON;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_voted_up() {
	user_type = SWC::USER_UGC_LIST_VOTED_UP;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_voted_down() {
	user_type = SWC::USER_UGC_LIST_VOTED_DOWN;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_will_vote_later() {
	user_type = SWC::USER_UGC_LIST_WILL_VOTE_LATER;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_favorited() {
	user_type = SWC::USER_UGC_LIST_FAVORITED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_subscribed() {
	user_type = SWC::USER_UGC_LIST_SUBSCRIBED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_used_or_played() {
	user_type = SWC::USER_UGC_LIST_USED_OR_PLAYED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_user_followed() {
	user_type = SWC::USER_UGC_LIST_FOLLOWED;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_creation_date() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_CREATION_ORDER_DESC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_creation_date_asc() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_CREATION_ORDER_ASC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_title_asc() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_TITLE_ASC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_update_date() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_LAST_UPDATED_DESC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_subscription_date() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_SUBSCRIPTION_DATE_DESC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_vote_score() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_VOTE_SCORE_DESC;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::sort_by_moderation() {
	user_sort = SWC::USER_UGC_LIST_SORT_ORDER_FOR_MODERATION;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::where_search_text(const String &p_search_text) {
	search_text = p_search_text;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_file_ids(const Vector<int64_t> &p_file_ids) {
	files.resize(p_file_ids.size());
	for (int i = 0; i < p_file_ids.size(); i++) {
		files.ptrw()[i] = p_file_ids[i];
	}
	return this;
}

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_user(const Ref<HBSteamFriend> &p_user) {
	user = p_user;
	return this;
}

// Shared stuff
Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_type(SWC::UGCMatchingUGCType p_matching_type) {
	matching_type = p_matching_type;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::allow_cached_response(int p_max_age_seconds) {
	max_cache_age = p_max_age_seconds;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::in_language(const String &p_language) {
	language = p_language;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_trend_days(int p_trend_days) {
	trend_days = p_trend_days;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::match_any_tag() {
	match_any_tag_value = true;
	has_match_any_tag = true;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::match_all_tags() {
	match_any_tag_value = false;
	has_match_any_tag = true;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_tag(const String &p_tag) {
	required_tags.push_back(p_tag);
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::add_required_key_value_tag(const String &p_key, const String &p_value) {
	required_kv[p_key] = p_value;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::without_tag(const String &p_tag) {
	excluded_tags.push_back(p_tag);
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_only_ids(bool p_with_only_ids) {
	wants_only_ids = p_with_only_ids;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_long_description(bool p_wants_long_description) {
	wants_long_description = p_wants_long_description;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_metadata(bool p_wants_metadata) {
	wants_metadata = p_wants_metadata;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_children(bool p_wants_children) {
	wants_children = p_wants_children;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_additional_previews(bool p_wants_additional_previews) {
	wants_additional_previews = p_wants_additional_previews;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_total_only(bool p_wants_total_only) {
	wants_total_only = p_wants_total_only;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_playtime_stats(bool p_wants_playtime_stats) {
	wants_playtime_stats = p_wants_playtime_stats;
	return this;
};

Ref<HBSteamUGCQuery> HBSteamUGCQuery::create_query(SWC::UGCMatchingUGCType p_matching_type) {
	return memnew(HBSteamUGCQuery(p_matching_type));
}

void HBSteamUGCQuery::request_page(int p_page) {
	Ref<HBSteamUGC> ugc = Steamworks::get_singleton()->get_ugc();
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	int creator_app = Steamworks::get_singleton()->get_app_id();
	int consumer_app = creator_app;

	QueryScopeType query_scope_type;

	if (files.size() > 0) {
		query_handle = SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest(iugc, (PublishedFileId_t *)files.ptr(), files.size());
		query_scope_type = QUERY_DETAILS;
	} else if (user.is_valid()) {
		query_handle = SteamAPI_ISteamUGC_CreateQueryUserUGCRequest(iugc, user->get_account_id(), (EUserUGCList)user_type, (EUGCMatchingUGCType)matching_type, (EUserUGCListSortOrder)user_sort, creator_app, consumer_app, p_page);
		query_scope_type = QUERY_USER;
	} else {
		query_handle = SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage(iugc, (EUGCQuery)query_type, (EUGCMatchingUGCType)matching_type, creator_app, consumer_app, p_page);
		query_scope_type = QUERY_ALL;
	}

	_apply_returns(query_scope_type);

	if (max_cache_age >= 0) {
		SteamAPI_ISteamUGC_SetAllowCachedResponse(iugc, query_handle, max_cache_age);
	}
	_apply_constraints(query_scope_type);

	SteamAPICall_t api_call = SteamAPI_ISteamUGC_SendQueryUGCRequest(iugc, query_handle);

	// We pass a page result info to the callback with some information
	// this is to ensure its consistent, otherwise the user could change the query
	// params before callback
	HBSteamUGCQueryPageResult::ResultPageInfo page_result = {
		.handle = query_handle,
		.data_cached = false,
		.returns_kv_tags = wants_key_value_tags,
		.returns_default_stats = true,
		.returns_metadata = wants_metadata,
		.returns_children = wants_children,
		.returns_additional_previews = wants_additional_previews,
		.result_count = 0,
		.total_results = 0,
		.page = p_page,
	};
	page_infos[query_handle] = page_result;
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCQuery::_on_query_completed).bind(p_page));
}

void HBSteamUGC::_on_item_downloaded(Ref<SteamworksCallbackData> p_callback) {
	const DownloadItemResult_t *item_downloaded = p_callback->get_data<DownloadItemResult_t>();
	emit_signal("item_downloaded", (uint64_t)item_downloaded->m_unAppID, (uint64_t)item_downloaded->m_nPublishedFileId);
}

void HBSteamUGC::_on_item_installed(Ref<SteamworksCallbackData> p_callback) {
	const ItemInstalled_t *item_installed = p_callback->get_data<ItemInstalled_t>();
	emit_signal("item_installed", (uint64_t)item_installed->m_unAppID, (uint64_t)item_installed->m_nPublishedFileId);
	if (item_installed->m_unAppID == (unsigned int)Steamworks::get_singleton()->get_app_id()) {
		Ref<HBSteamUGCItem> item = HBSteamUGCItem::from_id(item_installed->m_nPublishedFileId);
		if (item.is_valid()) {
			item->_notify_item_installed(OK);
		}
	}
}

void HBSteamUGC::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &HBSteamUGC::is_valid);
	ClassDB::bind_method(D_METHOD("get_subscribed_items"), &HBSteamUGC::get_subscribed_items);
	ADD_SIGNAL(MethodInfo("item_installed", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "item_id")));
	ADD_SIGNAL(MethodInfo("item_downloaded", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "item_id")));
}

TypedArray<HBSteamUGCItem> HBSteamUGC::get_subscribed_items() {
	Vector<PublishedFileId_t> file_ids;
	TypedArray<HBSteamUGCItem> items;
	file_ids.resize(SteamAPI_ISteamUGC_GetNumSubscribedItems(steam_ugc));
	int items_returned = SteamAPI_ISteamUGC_GetSubscribedItems(steam_ugc, file_ids.ptrw(), file_ids.size());
	items.resize(items_returned);
	for (int i = 0; i < items_returned; i++) {
		items.set(i, HBSteamUGCItem::from_id(file_ids[i]));
	}
	return items;
}

void HBSteamUGC::init_interface() {
	steam_ugc = SteamAPI_SteamUGC();
	Steamworks::get_singleton()->add_callback(ItemInstalled_t::k_iCallback, callable_mp(this, &HBSteamUGC::_on_item_installed));
	Steamworks::get_singleton()->add_callback(DownloadItemResult_t::k_iCallback, callable_mp(this, &HBSteamUGC::_on_item_downloaded));
}

bool HBSteamUGC::is_valid() const {
	return steam_ugc != nullptr;
}

ISteamUGC *HBSteamUGC::get_interface() const {
	return steam_ugc;
}

void HBSteamUGCQueryPageResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_results"), &HBSteamUGCQueryPageResult::get_results_godot);
	ClassDB::bind_method(D_METHOD("get_total_results"), &HBSteamUGCQueryPageResult::get_total_results);
	ClassDB::bind_method(D_METHOD("get_page"), &HBSteamUGCQueryPageResult::get_page);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "results", PROPERTY_HINT_ARRAY_TYPE, "HBSteamUGCItem"), "", "get_results");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "total_results"), "", "get_total_results");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "page"), "", "get_page");
}

TypedArray<HBSteamUGCItem> HBSteamUGCQueryPageResult::get_results_godot() {
	Vector<Ref<HBSteamUGCItem>> ugc_items = get_results();
	TypedArray<HBSteamUGCItem> gd_ugc_items;
	gd_ugc_items.resize(ugc_items.size());
	for (int i = 0; i < ugc_items.size(); i++) {
		gd_ugc_items[i] = ugc_items[i];
	}
	return gd_ugc_items;
}

Vector<Ref<HBSteamUGCItem>> HBSteamUGCQueryPageResult::get_results() {
	if (results_cache.size() == page_info.result_count) {
		return results_cache;
	}
	results_cache.clear();

	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	for (int i = 0; i < page_info.result_count; i++) {
		SWC::SteamUGCDetails_t details;

		bool result = SteamAPI_ISteamUGC_GetQueryUGCResult(iugc, page_info.handle, i, (SteamUGCDetails_t *)&details);

		if (!result || details.result != SWC::RESULT_OK) {
			continue;
		}

		Ref<HBSteamUGCItem> item = HBSteamUGCItem::from_details(details);

		char preview_url[256];
		if (SteamAPI_ISteamUGC_GetQueryUGCPreviewURL(iugc, query_handle, i, preview_url, 256)) {
			item->preview_image_url = String::utf8(preview_url);
		}

		if (page_info.returns_kv_tags) {
			for (uint32_t j = 0; j < SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags(iugc, query_handle, i); j++) {
				char key[256];
				char value[256];
				bool ok = SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag(iugc, query_handle, i, j, key, 256, value, 256);
				if (!ok) {
					continue;
				}
				item->key_value_tags[String::utf8(key)] = String::utf8(value);
			}
		}
		if (page_info.returns_metadata) {
			char metadata_raw[k_cchDeveloperMetadataMax];
			if (SteamAPI_ISteamUGC_GetQueryUGCMetadata(iugc, query_handle, i, metadata_raw, k_cchDeveloperMetadataMax)) {
				item->metadata = String::utf8(metadata_raw);
			}
		}
		if (page_info.returns_children && details.num_children > 0) {
			Vector<PublishedFileId_t> children;
			children.resize(details.num_children);

			if (SteamAPI_ISteamUGC_GetQueryUGCChildren(iugc, query_handle, i, children.ptrw(), details.num_children)) {
				TypedArray<int64_t> children_gd;
				children_gd.resize(details.num_children);
				for (uint32_t j = 0; j < details.num_children; j++) {
					int64_t val = children.ptr()[j];
					children_gd[j] = val;
				}
				item->children = children_gd;
			}
		}

		if (page_info.returns_additional_previews) {
			uint32_t preview_count = SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews(iugc, query_handle, i);
			char url_or_video_id[256];
			char original_filename[256];
			SWC::ItemPreviewType preview_type;
			for (uint32_t j = 0; j < preview_count; j++) {
				bool ok = SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview(iugc, query_handle, i, j, url_or_video_id, 256, original_filename, 256, (EItemPreviewType *)&preview_type);
				if (!ok) {
					continue;
				}
				item->additional_previews.push_back(memnew(HBSteamUGCAdditionalPreview(String::utf8(url_or_video_id), String::utf8(original_filename), preview_type)));
			}
		}

		results_cache.push_back(item);
	}
	return results_cache;
}

int HBSteamUGCQueryPageResult::get_total_results() const {
	return page_info.total_results;
}

int HBSteamUGCQueryPageResult::get_page() const {
	return page_info.page;
}

HBSteamUGCQueryPageResult::HBSteamUGCQueryPageResult(const ResultPageInfo &p_page_info) {
	query_handle = p_page_info.handle;

	page_info = p_page_info;
}

HBSteamUGCQueryPageResult::~HBSteamUGCQueryPageResult() {
	if (query_handle != SWC::UGC_QUERY_HANDLE_INVALID) {
		ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
		SteamAPI_ISteamUGC_ReleaseQueryUGCRequest(iugc, query_handle);
	}
}

HashMap<SWC::PublishedFileId_t, Ref<WeakRef>> HBSteamUGCItem::item_cache = HashMap<SWC::PublishedFileId_t, Ref<WeakRef>>();

void HBSteamUGCItem::_on_get_user_item_vote(Ref<SteamworksCallbackData> p_callback, bool p_io_failure) {
	const GetUserItemVoteResult_t *result = p_callback->get_data<GetUserItemVoteResult_t>();
	Ref<HBSteamUGCUserItemVoteResult> vote_result;
	vote_result.instantiate();
	vote_result->vote_down = result->m_bVotedDown;
	vote_result->vote_up = result->m_bVotedUp;
	vote_result->vote_skipped = result->m_bVoteSkipped;
	emit_signal("user_item_vote_received", vote_result);
}

void HBSteamUGCItem::_on_added_dependency(Ref<SteamworksCallbackData> p_callback, bool p_io_failure) {
	const AddUGCDependencyResult_t *result = p_callback->get_data<AddUGCDependencyResult_t>();
	emit_signal("dependency_added", (uint64_t)result->m_nChildPublishedFileId);
}

void HBSteamUGCItem::_on_removed_dependency(Ref<SteamworksCallbackData> p_callback, bool p_io_failure) {
	const RemoveUGCDependencyResult_t *result = p_callback->get_data<RemoveUGCDependencyResult_t>();
	emit_signal("dependency_removed", (uint64_t)result->m_nChildPublishedFileId);
}

void HBSteamUGCItem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_title"), &HBSteamUGCItem::get_title);
	ClassDB::bind_method(D_METHOD("get_description"), &HBSteamUGCItem::get_description);
	ClassDB::bind_method(D_METHOD("get_preview_image_url"), &HBSteamUGCItem::get_preview_image_url);
	ClassDB::bind_method(D_METHOD("get_item_id"), &HBSteamUGCItem::get_item_id);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &HBSteamUGCItem::has_tag);
	ClassDB::bind_method(D_METHOD("get_creator_app"), &HBSteamUGCItem::get_creator_app);
	ClassDB::bind_method(D_METHOD("get_consumer_app"), &HBSteamUGCItem::get_consumer_app);
	ClassDB::bind_method(D_METHOD("get_owner"), &HBSteamUGCItem::get_owner);
	ClassDB::bind_method(D_METHOD("get_score"), &HBSteamUGCItem::get_score);
	ClassDB::bind_method(D_METHOD("get_time_created"), &HBSteamUGCItem::get_time_created);
	ClassDB::bind_method(D_METHOD("get_time_updated"), &HBSteamUGCItem::get_time_updated);
	ClassDB::bind_method(D_METHOD("get_time_added_to_user_list"), &HBSteamUGCItem::get_time_added_to_user_list);
	ClassDB::bind_method(D_METHOD("get_visibility"), &HBSteamUGCItem::get_visibility);
	ClassDB::bind_method(D_METHOD("get_is_banned"), &HBSteamUGCItem::get_is_banned);
	ClassDB::bind_method(D_METHOD("get_votes_up"), &HBSteamUGCItem::get_votes_up);
	ClassDB::bind_method(D_METHOD("get_votes_down"), &HBSteamUGCItem::get_votes_down);
	ClassDB::bind_method(D_METHOD("get_children"), &HBSteamUGCItem::get_children);
	ClassDB::bind_method(D_METHOD("get_additional_previews"), &HBSteamUGCItem::get_additional_previews_godot);
	ClassDB::bind_method(D_METHOD("get_kv_tags"), &HBSteamUGCItem::get_kv_tags);
	ClassDB::bind_method(D_METHOD("edit"), &HBSteamUGCItem::edit);
	ClassDB::bind_method(D_METHOD("get_item_state"), &HBSteamUGCItem::get_item_state);
	ClassDB::bind_method(D_METHOD("get_metadata"), &HBSteamUGCItem::get_metadata);
	ClassDB::bind_method(D_METHOD("subscribe"), &HBSteamUGCItem::subscribe);
	ClassDB::bind_method(D_METHOD("unsubscribe"), &HBSteamUGCItem::unsubscribe);
	ClassDB::bind_method(D_METHOD("get_install_directory"), &HBSteamUGCItem::get_install_directory);
	ClassDB::bind_method(D_METHOD("download", "high_priorty"), &HBSteamUGCItem::download);
	ClassDB::bind_method(D_METHOD("request_user_vote"), &HBSteamUGCItem::request_user_vote);
	ClassDB::bind_method(D_METHOD("add_dependency", "child_id"), &HBSteamUGCItem::add_dependency);
	ClassDB::bind_method(D_METHOD("remove_dependency", "child_id"), &HBSteamUGCItem::remove_dependency);
	ClassDB::bind_method(D_METHOD("delete_item"), &HBSteamUGCItem::delete_item);
	ClassDB::bind_method(D_METHOD("set_user_item_vote", "vote_up"), &HBSteamUGCItem::set_user_item_vote);

	ClassDB::bind_static_method("HBSteamUGCItem", D_METHOD("from_id", "steam_id"), &HBSteamUGCItem::from_id);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "preview_image_url"), "", "get_preview_image_url");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "item_id"), "", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "creator_app"), "", "get_creator_app");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "consumer_app"), "", "get_consumer_app");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "owner", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), "", "get_owner");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "score"), "", "get_score");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "time_created"), "", "get_time_created");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "time_updated"), "", "get_time_updated");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "time_added_to_user_list"), "", "get_time_added_to_user_list");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visibility"), "", "get_visibility");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_banned"), "", "get_is_banned");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "votes_up"), "", "get_votes_up");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "votes_down"), "", "get_votes_down");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_ARRAY_TYPE, "int"), "", "get_children");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "additional_previews", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamUGCAdditionalPreview"), "", "get_additional_previews");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "kv_tags"), "", "get_kv_tags");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "item_state"), "", "get_item_state");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "metadata"), "", "get_metadata");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "install_directory"), "", "get_install_directory");

	ADD_SIGNAL(MethodInfo("user_item_vote_received", PropertyInfo(Variant::OBJECT, "item_vote_result", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamUGCUserItemVoteResult")));
	ADD_SIGNAL(MethodInfo("dependency_removed", PropertyInfo(Variant::INT, "child_id")));
	ADD_SIGNAL(MethodInfo("dependency_added", PropertyInfo(Variant::INT, "child_id")));
	ADD_SIGNAL(MethodInfo("item_installed", PropertyInfo(Variant::INT, "result")));
}

void HBSteamUGCItem::_notify_item_installed(int p_result) {
	emit_signal(SNAME("item_installed"), p_result);
}

void HBSteamUGCItem::update_from_details(const SWC::SteamUGCDetails_t &p_details) {
	ugc_details = p_details;
	title = String::utf8(p_details.title);
	description = String::utf8(p_details.description);
	Vector<String> new_tags = String::utf8(p_details.tags).to_lower().split(",", false);
	tags = new_tags.size() == 0 ? tags : new_tags;
}

int64_t HBSteamUGCItem::get_item_id() const { return ugc_details.published_file_id; }

String HBSteamUGCItem::get_preview_image_url() const { return preview_image_url; }

String HBSteamUGCItem::get_description() const { return description; }

String HBSteamUGCItem::get_title() const { return title; }

bool HBSteamUGCItem::has_tag(const String &p_tag) const {
	return tags.has(p_tag.to_lower());
}

uint64_t HBSteamUGCItem::get_creator_app() const { return ugc_details.creator_app_id; };

uint64_t HBSteamUGCItem::get_consumer_app() const { return ugc_details.consumer_app_id; };

Ref<HBSteamFriend> HBSteamUGCItem::get_owner() const {
	return HBSteamFriend::from_steam_id(ugc_details.steam_id_owner);
};

float HBSteamUGCItem::get_score() const { return ugc_details.score; };

uint64_t HBSteamUGCItem::get_time_created() const { return ugc_details.created; };

uint64_t HBSteamUGCItem::get_time_updated() const { return ugc_details.updated; };

uint64_t HBSteamUGCItem::get_time_added_to_user_list() const {
	return ugc_details.added_to_user_list;
}

SWC::RemoteStoragePublishedFileVisibility HBSteamUGCItem::get_visibility() const { return ugc_details.visibility; };

bool HBSteamUGCItem::get_is_banned() const { return ugc_details.banned; };

bool HBSteamUGCItem::get_is_accepted_for_use() const { return ugc_details.accepted_for_use; };

int HBSteamUGCItem::get_votes_up() const { return ugc_details.votes_up; };

int HBSteamUGCItem::get_votes_down() const { return ugc_details.votes_down; };

TypedArray<int64_t> HBSteamUGCItem::get_children() const { return children; }

TypedArray<HBSteamUGCAdditionalPreview> HBSteamUGCItem::get_additional_previews_godot() const {
	TypedArray<HBSteamUGCAdditionalPreview> arr;
	arr.resize(additional_previews.size());
	for (int i = 0; i < additional_previews.size(); i++) {
		arr[i] = additional_previews[i];
	}
	return arr;
}

Vector<Ref<HBSteamUGCAdditionalPreview>> HBSteamUGCItem::get_additional_previews() const {
	return additional_previews;
}

Dictionary HBSteamUGCItem::get_kv_tags() const { return key_value_tags; };

BitField<SWC::ItemState> HBSteamUGCItem::get_item_state() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_GetItemState(iugc, ugc_details.published_file_id);
}

String HBSteamUGCItem::get_metadata() const {
	return metadata;
}

Ref<HBSteamUGCEditor> HBSteamUGCItem::edit() const {
	Ref<HBSteamUGCEditor> editor;
	editor.instantiate();
	editor->file_id = ugc_details.published_file_id;
	editor->app_id = ugc_details.consumer_app_id;
	return editor;
}

String HBSteamUGCItem::get_install_directory() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	uint64_t size_on_disk;
	uint32_t time_stamp;
	char installation_folder[256];
	bool success = SteamAPI_ISteamUGC_GetItemInstallInfo(iugc, ugc_details.published_file_id, (uint64 *)&size_on_disk, installation_folder, 256, &time_stamp);
	if (!success) {
		return "";
	}
	return String::utf8(installation_folder);
}

bool HBSteamUGCItem::subscribe() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_SubscribeItem(iugc, ugc_details.published_file_id);
}

bool HBSteamUGCItem::unsubscribe() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_UnsubscribeItem(iugc, ugc_details.published_file_id);
}

bool HBSteamUGCItem::download(bool p_high_priority) const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_DownloadItem(iugc, ugc_details.published_file_id, p_high_priority);
}

bool HBSteamUGCItem::request_user_vote() {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	SteamAPICall_t api_call = SteamAPI_ISteamUGC_GetUserItemVote(iugc, ugc_details.published_file_id);
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCItem::_on_get_user_item_vote));
	return api_call != k_uAPICallInvalid;
}

bool HBSteamUGCItem::set_user_item_vote(bool p_vote_up) const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	SteamAPICall_t api_call = SteamAPI_ISteamUGC_SetUserItemVote(iugc, ugc_details.published_file_id, p_vote_up);
	return api_call != k_uAPICallInvalid;
}

void HBSteamUGCItem::add_dependency(uint64_t p_dependency_id) {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	SteamAPICall_t api_call = SteamAPI_ISteamUGC_AddDependency(iugc, ugc_details.published_file_id, p_dependency_id);
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCItem::_on_added_dependency));
}

void HBSteamUGCItem::remove_dependency(uint64_t p_dependency_id) {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	SteamAPICall_t api_call = SteamAPI_ISteamUGC_RemoveDependency(iugc, ugc_details.published_file_id, p_dependency_id);
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCItem::_on_removed_dependency));
}

void HBSteamUGCItem::delete_item() {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	SteamAPI_ISteamUGC_DeleteItem(iugc, ugc_details.published_file_id);
}

Ref<HBSteamUGCItem> HBSteamUGCItem::from_id(uint64_t p_item_id) {
	if (item_cache.has(p_item_id)) {
		Ref<WeakRef> cached_item_ref = item_cache[p_item_id];
		Ref<HBSteamUGCItem> cached_item = cached_item_ref->get_ref();
		if (cached_item.is_valid()) {
			return cached_item;
		}
	}

	Ref<WeakRef> weak_ref;
	weak_ref.instantiate();
	Ref<HBSteamUGCItem> item;
	item.instantiate();
	item->ugc_details.creator_app_id = Steamworks::get_singleton()->get_app_id();
	item->ugc_details.consumer_app_id = item->ugc_details.creator_app_id;
	item->ugc_details.published_file_id = p_item_id;
	weak_ref->set_ref(item);
	item_cache.insert(p_item_id, weak_ref);
	return item;
}

Ref<HBSteamUGCItem> HBSteamUGCItem::from_details(const SWC::SteamUGCDetails_t &p_details) {
	Ref<HBSteamUGCItem> item = from_id(p_details.published_file_id);
	item->update_from_details(p_details);
	return item;
}

void HBSteamUGCAdditionalPreview::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_url_or_video_id"), &HBSteamUGCAdditionalPreview::get_url_or_video_id);
	ClassDB::bind_method(D_METHOD("get_original_filename"), &HBSteamUGCAdditionalPreview::get_original_filename);
	ClassDB::bind_method(D_METHOD("get_preview_type"), &HBSteamUGCAdditionalPreview::get_preview_type);
}

HBSteamUGCAdditionalPreview::HBSteamUGCAdditionalPreview(const String &p_url_or_video_id, const String &p_original_filename, const SWC::ItemPreviewType &p_preview_type) {
	url_or_video_id = p_url_or_video_id;
	original_filename = p_original_filename;
	preview_type = p_preview_type;
}

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_key_value_tags(bool p_with_key_value_tags) {
	wants_key_value_tags = true;
	return this;
};

void HBSteamUGCEditor::_submit_update() {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	uint64_t consumer_app_id = app_id;
	if (consumer_app_id == 0) {
		consumer_app_id = Steamworks::get_singleton()->get_app_id();
	}
	update_handle = SteamAPI_ISteamUGC_StartItemUpdate(iugc, consumer_app_id, file_id);

	if (kv_tags_to_add.size() > 0) {
		for (KeyValue<String, String> kv : kv_tags_to_add) {
			SteamAPI_ISteamUGC_AddItemKeyValueTag(iugc, update_handle, kv.key.utf8(), kv.value.utf8());
		}
	}
	if (kv_tags_to_remove.size() > 0) {
		for (String key : kv_tags_to_remove) {
			SteamAPI_ISteamUGC_RemoveItemKeyValueTags(iugc, update_handle, key.utf8());
		}
	}
	if (has_content) {
		SteamAPI_ISteamUGC_SetItemContent(iugc, update_handle, content.utf8());
	}
	if (has_description) {
		SteamAPI_ISteamUGC_SetItemDescription(iugc, update_handle, description.utf8());
	}
	if (has_visiblity) {
		SteamAPI_ISteamUGC_SetItemVisibility(iugc, update_handle, (ERemoteStoragePublishedFileVisibility)visibility);
	}
	if (has_metadata) {
		SteamAPI_ISteamUGC_SetItemMetadata(iugc, update_handle, metadata.utf8());
	}
	if (has_preview_file) {
		SteamAPI_ISteamUGC_SetItemPreview(iugc, update_handle, preview_file.utf8());
	}
	if (has_preview_video_id) {
		SteamAPI_ISteamUGC_AddItemPreviewVideo(iugc, update_handle, preview_video_id.utf8());
	}
	if (has_tags) {
		SteamParamStringArray_t steam_tags;
		steam_tags.m_nNumStrings = tags.size();
		steam_tags.m_ppStrings = memnew_arr(const char *, tags.size());
		// We need to keep the strings alive in memory
		Vector<CharString> strings;
		for (int i = 0; i < tags.size(); i++) {
			CharString cs = tags[i].utf8();
			strings.push_back(cs);
			steam_tags.m_ppStrings[i] = cs;
		}
		SteamAPI_ISteamUGC_SetItemTags(iugc, update_handle, &steam_tags);
		memdelete_arr(steam_tags.m_ppStrings);
	}
	if (has_title) {
		SteamAPI_ISteamUGC_SetItemTitle(iugc, update_handle, title.utf8());
	}

	SteamAPICall_t api_call;
	if (!has_changelog) {
		api_call = SteamAPI_ISteamUGC_SubmitItemUpdate(iugc, update_handle, nullptr);
	} else {
		api_call = SteamAPI_ISteamUGC_SubmitItemUpdate(iugc, update_handle, changelog.utf8());
	}
	if (api_call == k_uAPICallInvalid) {
		emit_signal("file_submitted", SWC::Result::RESULT_FAIL, false);
		return;
	}
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCEditor::_on_item_updated));
}

void HBSteamUGCEditor::_on_item_created(Ref<SteamworksCallbackData> p_callback, bool p_io_failure) {
	const CreateItemResult_t *result = p_callback->get_data<CreateItemResult_t>();
	if (result->m_eResult != EResult::k_EResultOK) {
		emit_signal("file_submitted", (SWC::Result)result->m_eResult, result->m_bUserNeedsToAcceptWorkshopLegalAgreement);
		return;
	}
	file_id = result->m_nPublishedFileId;
	_submit_update();
}

void HBSteamUGCEditor::_on_item_updated(Ref<SteamworksCallbackData> p_callback, bool p_io_failure) {
	const SubmitItemUpdateResult_t *update_result = p_callback->get_data<SubmitItemUpdateResult_t>();
	if (update_result->m_eResult != k_EResultOK) {
		emit_signal("file_submitted", (SWC::Result)update_result->m_eResult, false);
		return;
	}
	emit_signal("file_submitted", SWC::Result::RESULT_OK, update_result->m_bUserNeedsToAcceptWorkshopLegalAgreement);
}

void HBSteamUGCEditor::_bind_methods() {
	ClassDB::bind_static_method("HBSteamUGCEditor", D_METHOD("new_community_file"), &HBSteamUGCEditor::new_community_file);
	ClassDB::bind_method(D_METHOD("add_kv_tag", "key", "value"), &HBSteamUGCEditor::add_kv_tag);
	ClassDB::bind_method(D_METHOD("remove_kv_tag", "key"), &HBSteamUGCEditor::remove_kv_tag);
	ClassDB::bind_method(D_METHOD("for_app_id", "app_id"), &HBSteamUGCEditor::for_app_id);
	ClassDB::bind_method(D_METHOD("with_changelog", "changelog"), &HBSteamUGCEditor::with_changelog);
	ClassDB::bind_method(D_METHOD("with_description", "description"), &HBSteamUGCEditor::with_description);
	ClassDB::bind_method(D_METHOD("with_content", "content_path"), &HBSteamUGCEditor::with_content);
	ClassDB::bind_method(D_METHOD("with_visibility", "visiblity"), &HBSteamUGCEditor::with_visibility);
	ClassDB::bind_method(D_METHOD("with_metadata", "metadata"), &HBSteamUGCEditor::with_metadata);
	ClassDB::bind_method(D_METHOD("with_preview_file", "preview_file"), &HBSteamUGCEditor::with_preview_file);
	ClassDB::bind_method(D_METHOD("with_preview_video_id", "video_id"), &HBSteamUGCEditor::with_preview_video_id);
	ClassDB::bind_method(D_METHOD("with_tags", "tags"), &HBSteamUGCEditor::with_tags);
	ClassDB::bind_method(D_METHOD("with_title", "title"), &HBSteamUGCEditor::with_title);
	ClassDB::bind_method(D_METHOD("get_update_progress"), &HBSteamUGCEditor::get_update_progress);
	ClassDB::bind_method(D_METHOD("get_file_id"), &HBSteamUGCEditor::get_file_id);

	ClassDB::bind_method(D_METHOD("submit"), &HBSteamUGCEditor::submit);

	ADD_SIGNAL(MethodInfo("file_submitted", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "user_needs_to_accept_workshop_legal_agreement")));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "file_id"), "", "get_file_id");
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::add_kv_tag(const String &p_key, const String &p_value) {
	if (kv_tags_to_remove.has(p_key)) {
		kv_tags_to_remove.erase(p_key);
	}
	kv_tags_to_add.insert(p_key, p_value);
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::remove_kv_tag(const String &p_key) {
	if (kv_tags_to_add.has(p_key)) {
		kv_tags_to_add.erase(p_key);
	}
	kv_tags_to_remove.push_back(p_key);
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::for_app_id(uint64_t p_app_id) {
	app_id = p_app_id;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_changelog(const String &p_changelog) {
	has_changelog = true;
	changelog = p_changelog;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_description(const String &p_description) {
	has_description = true;
	description = p_description;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_content(const String &p_content_path) {
	has_content = true;
	content = p_content_path;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_visibility(SWC::RemoteStoragePublishedFileVisibility p_visibility) {
	has_visiblity = true;
	visibility = p_visibility;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_metadata(const String &p_metadata) {
	has_metadata = true;
	metadata = p_metadata;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_preview_file(const String &p_preview_file) {
	has_preview_file = true;
	preview_file = p_preview_file;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_preview_video_id(const String &p_video_id) {
	has_preview_video_id = true;
	preview_video_id = p_video_id;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_tags(Vector<String> p_tags) {
	has_tags = p_tags.size() != 0;
	tags = p_tags;
	return this;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::with_title(const String &p_title) {
	has_title = true;
	title = p_title;
	return this;
}

void HBSteamUGCEditor::submit() {
	if (!creating_new) {
		_submit_update();
		return;
	}
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	uint64_t consumer_app_id = app_id;
	if (consumer_app_id == 0) {
		consumer_app_id = Steamworks::get_singleton()->get_app_id();
	}
	SteamAPICall_t api_call = SteamAPI_ISteamUGC_CreateItem(iugc, consumer_app_id, EWorkshopFileType::k_EWorkshopFileTypeCommunity);
	if (api_call == k_uAPICallInvalid) {
		emit_signal("file_submitted", SWC::Result::RESULT_FAIL, false);
		return;
	}
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCEditor::_on_item_created));
}

Ref<HBSteamUGCItemUpdateProgress> HBSteamUGCEditor::get_update_progress() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	uint64_t bytes_processed, bytes_total;
	EItemUpdateStatus update_status = SteamAPI_ISteamUGC_GetItemUpdateProgress(iugc, update_handle, (uint64 *)&bytes_processed, (uint64 *)&bytes_total);
	Ref<HBSteamUGCItemUpdateProgress> update_progress;
	update_progress.instantiate();
	update_progress->update_status = (SWC::ItemUpdateStatus)update_status;
	update_progress->bytes_processed = bytes_processed;
	update_progress->bytes_total = bytes_total;
	return update_progress;
}

Ref<HBSteamUGCEditor> HBSteamUGCEditor::new_community_file() {
	Ref<HBSteamUGCEditor> editor;
	editor.instantiate();
	editor->creating_new = true;
	return editor;
}

uint64_t HBSteamUGCEditor::get_file_id() const {
	return file_id;
}

void HBSteamUGCUserItemVoteResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_vote_skipped"), &HBSteamUGCUserItemVoteResult::get_vote_skipped);
	ClassDB::bind_method(D_METHOD("get_vote_up"), &HBSteamUGCUserItemVoteResult::get_vote_up);
	ClassDB::bind_method(D_METHOD("get_vote_down"), &HBSteamUGCUserItemVoteResult::get_vote_down);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vote_up"), "", "get_vote_up");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vote_skipped"), "", "get_vote_skipped");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vote_down"), "", "get_vote_down");
}

bool HBSteamUGCUserItemVoteResult::get_vote_up() const { return vote_up; }

bool HBSteamUGCUserItemVoteResult::get_vote_down() const { return vote_down; }

bool HBSteamUGCUserItemVoteResult::get_vote_skipped() const { return vote_skipped; }

void HBSteamUGCItemUpdateProgress::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_update_status"), &HBSteamUGCItemUpdateProgress::get_update_status);
	ClassDB::bind_method(D_METHOD("get_bytes_total"), &HBSteamUGCItemUpdateProgress::get_bytes_total);
	ClassDB::bind_method(D_METHOD("get_bytes_processed"), &HBSteamUGCItemUpdateProgress::get_bytes_processed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "update_status"), "", "get_update_status");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bytes_total"), "", "get_bytes_total");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bytes_processed"), "", "get_bytes_processed");
}

SWC::ItemUpdateStatus HBSteamUGCItemUpdateProgress::get_update_status() const {
	return update_status;
}

uint64_t HBSteamUGCItemUpdateProgress::get_bytes_total() const {
	return bytes_total;
}

uint64_t HBSteamUGCItemUpdateProgress::get_bytes_processed() const {
	return bytes_processed;
}
