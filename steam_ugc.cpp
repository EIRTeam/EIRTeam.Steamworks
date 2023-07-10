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
	emit_signal("query_completed", p_page, page_result);
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

	ADD_SIGNAL(MethodInfo("query_completed", PropertyInfo(Variant::INT, "page"), PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamUGCQueryPageResult")));
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
		query_handle,
		false,
		wants_key_value_tags,
		true,
		wants_metadata,
		wants_children,
		wants_additional_previews

	};
	page_infos[query_handle] = page_result;
	Steamworks::get_singleton()->add_call_result_callback(api_call, callable_mp(this, &HBSteamUGCQuery::_on_query_completed).bind(p_page));
}

void HBSteamUGC::_on_item_installed(Ref<SteamworksCallbackData> p_callback) {
	const ItemInstalled_t *item_installed = p_callback->get_data<ItemInstalled_t>();
	emit_signal("item_installed", (uint64_t)item_installed->m_unAppID, (uint64_t)item_installed->m_nPublishedFileId);
}

void HBSteamUGC::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_valid"), &HBSteamUGC::is_valid);
	ADD_SIGNAL(MethodInfo("item_installed", PropertyInfo(Variant::INT, "app_id"), PropertyInfo(Variant::INT, "item_id")));
}

Vector<Ref<HBSteamUGCItem>> HBSteamUGC::get_subscribed_items() {
	Vector<PublishedFileId_t> file_ids;
	file_ids.resize(SteamAPI_ISteamUGC_GetNumSubscribedItems(steam_ugc));
	int items_returned = SteamAPI_ISteamUGC_GetSubscribedItems(steam_ugc, file_ids.ptrw(), file_ids.size());
	Vector<Ref<HBSteamUGCItem>> items;
	items.resize(items_returned);
	for (int i = 0; i < items_returned; i++) {
		items.ptrw()[i] = HBSteamUGCItem::from_id(file_ids[i]);
	}
	return items;
}

void HBSteamUGC::init_interface() {
	steam_ugc = SteamAPI_SteamUGC();
	Steamworks::get_singleton()->add_callback(ItemInstalled_t::k_iCallback, callable_mp(this, &HBSteamUGC::_on_item_installed));
}

bool HBSteamUGC::is_valid() const {
	return steam_ugc != nullptr;
}

ISteamUGC *HBSteamUGC::get_interface() const {
	return steam_ugc;
}

void HBSteamUGCQueryPageResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_results"), &HBSteamUGCQueryPageResult::get_results_godot);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "results", PROPERTY_HINT_ARRAY_TYPE, "HBsteamUGCItem"), "", "get_results");
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

		if (!result) {
			continue;
		}

		SWC::SteamUGCDetails_t *ugc_details = (SWC::SteamUGCDetails_t *)&details;
		Ref<HBSteamUGCItem> item = HBSteamUGCItem::from_details(*ugc_details);

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
	ClassDB::bind_method(D_METHOD("get_time_updated"), &HBSteamUGCItem::get_time_updated);
	ClassDB::bind_method(D_METHOD("get_visibility"), &HBSteamUGCItem::get_visibility);
	ClassDB::bind_method(D_METHOD("get_is_banned"), &HBSteamUGCItem::get_is_banned);
	ClassDB::bind_method(D_METHOD("get_votes_up"), &HBSteamUGCItem::get_votes_up);
	ClassDB::bind_method(D_METHOD("get_votes_down"), &HBSteamUGCItem::get_votes_down);
	ClassDB::bind_method(D_METHOD("get_children"), &HBSteamUGCItem::get_children);
	ClassDB::bind_method(D_METHOD("get_additional_previews"), &HBSteamUGCItem::get_additional_previews_godot);
	ClassDB::bind_method(D_METHOD("get_kv_tags"), &HBSteamUGCItem::get_kv_tags);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "preview_image_url"), "", "get_preview_image_url");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "item_id"), "", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "creator_app"), "", "get_creator_app");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "consumer_app"), "", "get_consumer_app");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "owner", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamFriend"), "", "get_owner");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "score"), "", "get_score");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "time_updated"), "", "get_time_updated");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visibility"), "", "get_visibility");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_banned"), "", "get_is_banned");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "votes_up"), "", "get_votes_up");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "votes_down"), "", "get_votes_down");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_ARRAY_TYPE, "int"), "", "get_children");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "additional_previews", PROPERTY_HINT_RESOURCE_TYPE, "HBSteamUGCAdditionalPreview"), "", "get_additional_previews");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "kv_tags"), "", "get_kv_tags");
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

Ref<HBSteamFriend> HBSteamUGCItem::get_owner() const { return HBSteamFriend::from_steam_id(ugc_details.steam_id_owner); };

float HBSteamUGCItem::get_score() const { return ugc_details.score; };

uint64_t HBSteamUGCItem::get_time_created() const { return ugc_details.created; };

uint64_t HBSteamUGCItem::get_time_updated() const { return ugc_details.updated; };

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

bool HBSteamUGCItem::subscribe() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_SubscribeItem(iugc, ugc_details.published_file_id);
}

bool HBSteamUGCItem::unsubscribe() const {
	ISteamUGC *iugc = Steamworks::get_singleton()->get_ugc()->get_interface();
	return SteamAPI_ISteamUGC_UnsubscribeItem(iugc, ugc_details.published_file_id);
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

Ref<HBSteamUGCQuery> HBSteamUGCQuery::with_key_value_tags(bool p_with_key_value_tags) {
	wants_key_value_tags = true;
	return this;
};
