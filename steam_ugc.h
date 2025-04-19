/**************************************************************************/
/*  steam_ugc.h                                                           */
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

#ifndef STEAM_UGC_H
#define STEAM_UGC_H

#include "core/object/ref_counted.h"
#include "steam_friends.h"
#include "steamworks_callback_data.h"
#include "steamworks_constants.gen.h"

class ISteamUGC;
class HBSteamUGCQueryPageResult;
class HBSteamUGCItem;
class HBSteamUGCEditor;

class HBSteamUGCItemUpdateProgress : public RefCounted {
	GDCLASS(HBSteamUGCItemUpdateProgress, RefCounted);
	uint64_t bytes_processed;
	uint64_t bytes_total;
	SWC::ItemUpdateStatus update_status;

protected:
	static void _bind_methods();

public:
	SWC::ItemUpdateStatus get_update_status() const;
	uint64_t get_bytes_total() const;
	uint64_t get_bytes_processed() const;
	friend class HBSteamUGCEditor;
};

class HBSteamUGCUserItemVoteResult : public RefCounted {
	GDCLASS(HBSteamUGCUserItemVoteResult, RefCounted);
	bool vote_up;
	bool vote_down;
	bool vote_skipped;

protected:
	static void _bind_methods();

public:
	bool get_vote_up() const;
	bool get_vote_down() const;
	bool get_vote_skipped() const;

	friend class HBSteamUGCItem;
};

class HBSteamUGCPublishResult : public RefCounted {
	GDCLASS(HBSteamUGCPublishResult, RefCounted);
};

class HBSteamUGCEditor : public RefCounted {
	GDCLASS(HBSteamUGCEditor, RefCounted);
	bool creating_new = false;
	uint64_t app_id = 0;
	uint64_t file_id = 0;
	SWC::UGCUpdateHandle_t update_handle = SWC::UGC_UPDATE_HANDLE_INVALID;

	HashMap<String, String> kv_tags_to_add;
	Vector<String> kv_tags_to_remove;
	bool has_changelog = false;
	String changelog;
	bool has_content = false;
	String content;
	bool has_description = false;
	String description;
	bool has_visiblity = false;
	SWC::RemoteStoragePublishedFileVisibility visibility;
	bool has_metadata = false;
	String metadata;
	bool has_preview_file = false;
	String preview_file;
	bool has_preview_video_id = false;
	String preview_video_id;
	bool has_tags = false;
	Vector<String> tags;
	bool has_title = false;
	String title;
	void _submit_update();
	void _on_item_created(Ref<SteamworksCallbackData> p_callback, bool p_io_failure);
	void _on_item_updated(Ref<SteamworksCallbackData> p_callback, bool p_io_failure);

protected:
	static void _bind_methods();

public:
	Ref<HBSteamUGCEditor> add_kv_tag(const String &p_key, const String &p_value);
	Ref<HBSteamUGCEditor> remove_kv_tag(const String &p_key);
	Ref<HBSteamUGCEditor> for_app_id(uint64_t p_app_id);
	Ref<HBSteamUGCEditor> with_changelog(const String &p_changelog);
	Ref<HBSteamUGCEditor> with_description(const String &p_description);
	Ref<HBSteamUGCEditor> with_content(const String &p_content_path);
	Ref<HBSteamUGCEditor> with_visibility(SWC::RemoteStoragePublishedFileVisibility p_visibility);
	Ref<HBSteamUGCEditor> with_metadata(const String &p_metadata);
	Ref<HBSteamUGCEditor> with_preview_file(const String &p_preview_file);
	Ref<HBSteamUGCEditor> with_preview_video_id(const String &p_video_id);
	Ref<HBSteamUGCEditor> with_tags(Vector<String> p_tags);

	Ref<HBSteamUGCEditor> with_title(const String &p_title);
	void submit();
	Ref<HBSteamUGCItemUpdateProgress> get_update_progress() const;
	static Ref<HBSteamUGCEditor> new_community_file();
	uint64_t get_file_id() const;
	friend class HBSteamUGCItem;
};

class HBSteamUGCAdditionalPreview : public RefCounted {
	GDCLASS(HBSteamUGCAdditionalPreview, RefCounted);
	String url_or_video_id;
	String original_filename;
	SWC::ItemPreviewType preview_type;

protected:
	static void _bind_methods();

public:
	String get_url_or_video_id() const { return url_or_video_id; };
	String get_original_filename() const { return original_filename; };
	SWC::ItemPreviewType get_preview_type() const { return preview_type; };

	HBSteamUGCAdditionalPreview(const String &p_url_or_video_id, const String &p_original_filename, const SWC::ItemPreviewType &p_preview_type);
};

class HBSteamUGCItem : public RefCounted {
	GDCLASS(HBSteamUGCItem, RefCounted);
	static HashMap<SWC::PublishedFileId_t, HBSteamUGCItem*> item_cache;
	SWC::SteamUGCDetails_t ugc_details;
	String title;
	String description;
	String preview_image_url;
	String metadata;
	Vector<String> tags;
	TypedArray<int64_t> children;
	Vector<Ref<HBSteamUGCAdditionalPreview>> additional_previews;
	Dictionary key_value_tags;
	void _on_get_user_item_vote(Ref<SteamworksCallbackData> p_callback, bool p_io_failure);
	void _on_added_dependency(Ref<SteamworksCallbackData> p_callback, bool p_io_failure);
	void _on_removed_dependency(Ref<SteamworksCallbackData> p_callback, bool p_io_failure);

protected:
	static void _bind_methods();
	void _notify_item_installed(int p_result);

public:
	void update_from_details(const SWC::SteamUGCDetails_t &p_details);
	String get_title() const;
	String get_description() const;
	String get_preview_image_url() const;
	int64_t get_item_id() const;
	bool has_tag(const String &p_tag) const;
	uint64_t get_creator_app() const;
	uint64_t get_consumer_app() const;
	Ref<HBSteamFriend> get_owner() const;
	float get_score() const;
	uint64_t get_time_created() const;
	uint64_t get_time_updated() const;
	uint64_t get_time_added_to_user_list() const;
	SWC::RemoteStoragePublishedFileVisibility get_visibility() const;
	bool get_is_banned() const;
	bool get_is_accepted_for_use() const;
	int get_votes_up() const;
	int get_votes_down() const;
	TypedArray<int64_t> get_children() const;
	TypedArray<HBSteamUGCAdditionalPreview> get_additional_previews_godot() const;
	Vector<Ref<HBSteamUGCAdditionalPreview>> get_additional_previews() const;
	Dictionary get_kv_tags() const;
	BitField<SWC::ItemState> get_item_state() const;
	String get_metadata() const;
	Ref<HBSteamUGCEditor> edit() const;
	String get_install_directory() const;

	bool subscribe() const;
	bool unsubscribe() const;

	bool download(bool p_high_priority) const;
	bool request_user_vote();
	bool set_user_item_vote(bool p_vote_up) const;

	void add_dependency(uint64_t p_dependency);
	void remove_dependency(uint64_t p_dependency);
	void delete_item();

	static Ref<HBSteamUGCItem> from_id(uint64_t p_item_id);
	static Ref<HBSteamUGCItem> from_details(const SWC::SteamUGCDetails_t &p_details);
	~HBSteamUGCItem();
	friend class HBSteamUGCQueryPageResult;
	friend class HBSteamUGC;
};

class HBSteamUGCQueryPageResult : public RefCounted {
	GDCLASS(HBSteamUGCQueryPageResult, RefCounted);
	SWC::UGCQueryHandle_t query_handle = SWC::UGC_QUERY_HANDLE_INVALID;
	Vector<Ref<HBSteamUGCItem>> results_cache;

public:
	struct ResultPageInfo {
		SWC::UGCQueryHandle_t handle;
		bool data_cached;

		bool returns_kv_tags;
		bool returns_default_stats;
		bool returns_metadata;
		bool returns_children;
		bool returns_additional_previews;
		int result_count;
		int total_results;
		int page;
	};

private:
	ResultPageInfo page_info;

protected:
	static void _bind_methods();

public:
	TypedArray<HBSteamUGCItem> get_results_godot();
	Vector<Ref<HBSteamUGCItem>> get_results();
	int get_total_results() const;
	int get_page() const;
	HBSteamUGCQueryPageResult(const ResultPageInfo &p_page_info);
	~HBSteamUGCQueryPageResult();
};

class HBSteamUGCQuery : public RefCounted {
	GDCLASS(HBSteamUGCQuery, RefCounted);
	SWC::UGCQueryHandle_t query_handle = SWC::UGC_QUERY_HANDLE_INVALID;
	SWC::UGCMatchingUGCType matching_type;
	SWC::UGCQuery query_type = SWC::UGCQuery::UGC_QUERY_RANKED_BY_VOTE;
	String search_text;
	HashMap<SWC::UGCQueryHandle_t, HBSteamUGCQueryPageResult::ResultPageInfo> page_infos;
	// User query shit

	Ref<HBSteamFriend> user;
	SWC::UserUGCList user_type = SWC::USER_UGC_LIST_PUBLISHED;
	SWC::UserUGCListSortOrder user_sort = SWC::UserUGCListSortOrder::USER_UGC_LIST_SORT_ORDER_CREATION_ORDER_DESC;

	Vector<uint64_t> files;
	int max_cache_age = -1;
	String language;
	int trend_days = -1;
	bool has_match_any_tag = false;
	Vector<String> required_tags;
	bool match_any_tag_value = false;
	Vector<String> excluded_tags;
	HashMap<String, String> required_kv;

	bool wants_only_ids = false;
	bool wants_key_value_tags = false;
	bool wants_long_description = false;
	bool wants_metadata = false;
	bool wants_children = false;
	bool wants_additional_previews = false;
	bool wants_total_only = false;
	int wants_playtime_stats = -1;

private:
	enum QueryScopeType {
		QUERY_ALL,
		QUERY_DETAILS,
		QUERY_USER
	};
	void _apply_returns(QueryScopeType p_query_type);
	void _apply_constraints(QueryScopeType p_query_type);
	void _on_query_completed(Ref<SteamworksCallbackData> p_callback, bool p_io_failure, int p_page);
	static void _bind_methods();

public:
	HBSteamUGCQuery(SWC::UGCMatchingUGCType p_ugc_type);
	Ref<HBSteamUGCQuery> ranked_by_vote();
	Ref<HBSteamUGCQuery> ranked_by_publication_date();
	Ref<HBSteamUGCQuery> ranked_by_acceptance_date();
	Ref<HBSteamUGCQuery> ranked_by_trend();
	Ref<HBSteamUGCQuery> favorited_by_friends();
	Ref<HBSteamUGCQuery> created_by_friends();
	Ref<HBSteamUGCQuery> ranked_by_num_times_reported();
	Ref<HBSteamUGCQuery> created_by_followed_users();
	Ref<HBSteamUGCQuery> not_yet_rated();
	Ref<HBSteamUGCQuery> ranked_by_total_votes_asc();
	Ref<HBSteamUGCQuery> ranked_by_text_search();
	Ref<HBSteamUGCQuery> ranked_by_total_unique_subscriptions();
	Ref<HBSteamUGCQuery> ranked_by_playtime_trend();
	Ref<HBSteamUGCQuery> ranked_by_total_playtime();
	Ref<HBSteamUGCQuery> ranked_by_average_playtime_trend();
	Ref<HBSteamUGCQuery> ranked_by_lifetime_average_playtime();
	Ref<HBSteamUGCQuery> ranked_by_playtime_sessions_trend();
	Ref<HBSteamUGCQuery> ranked_by_lifetime_playtime_sessions();

	// User query shit
	Ref<HBSteamUGCQuery> where_user_published();
	Ref<HBSteamUGCQuery> where_user_voted_on();
	Ref<HBSteamUGCQuery> where_user_voted_up();
	Ref<HBSteamUGCQuery> where_user_voted_down();
	Ref<HBSteamUGCQuery> where_user_will_vote_later();
	Ref<HBSteamUGCQuery> where_user_favorited();
	Ref<HBSteamUGCQuery> where_user_subscribed();
	Ref<HBSteamUGCQuery> where_user_used_or_played();
	Ref<HBSteamUGCQuery> where_user_followed();

	Ref<HBSteamUGCQuery> sort_by_creation_date();
	Ref<HBSteamUGCQuery> sort_by_creation_date_asc();
	Ref<HBSteamUGCQuery> sort_by_title_asc();
	Ref<HBSteamUGCQuery> sort_by_update_date();
	Ref<HBSteamUGCQuery> sort_by_subscription_date();
	Ref<HBSteamUGCQuery> sort_by_vote_score();
	Ref<HBSteamUGCQuery> sort_by_moderation();

	Ref<HBSteamUGCQuery> where_search_text(const String &p_search_text);

	Ref<HBSteamUGCQuery> with_file_ids(const Vector<int64_t> &p_file_ids);

	Ref<HBSteamUGCQuery> with_user(const Ref<HBSteamFriend> &p_user);

	Ref<HBSteamUGCQuery> with_type(SWC::UGCMatchingUGCType p_matching_type);
	Ref<HBSteamUGCQuery> allow_cached_response(int p_max_age_seconds);
	Ref<HBSteamUGCQuery> in_language(const String &p_language);
	Ref<HBSteamUGCQuery> with_trend_days(int p_trend_days);

	Ref<HBSteamUGCQuery> match_any_tag();
	Ref<HBSteamUGCQuery> match_all_tags();
	Ref<HBSteamUGCQuery> with_tag(const String &p_tag);
	Ref<HBSteamUGCQuery> add_required_key_value_tag(const String &p_key, const String &p_value);
	Ref<HBSteamUGCQuery> without_tag(const String &p_tag);

	Ref<HBSteamUGCQuery> with_key_value_tags(bool p_with_key_value_tags);
	Ref<HBSteamUGCQuery> with_only_ids(bool p_with_only_ids);
	Ref<HBSteamUGCQuery> with_long_description(bool p_wants_long_description);
	Ref<HBSteamUGCQuery> with_metadata(bool p_wants_metadata);
	Ref<HBSteamUGCQuery> with_children(bool p_wants_children);
	Ref<HBSteamUGCQuery> with_additional_previews(bool p_wants_additional_previews);
	Ref<HBSteamUGCQuery> with_total_only(bool p_wants_total_only);
	Ref<HBSteamUGCQuery> with_playtime_stats(bool p_wants_playtime_stats);

	static Ref<HBSteamUGCQuery> create_query(SWC::UGCMatchingUGCType p_matching_type);
	void request_page(int p_page);
};

class HBSteamUGC : public RefCounted {
	GDCLASS(HBSteamUGC, RefCounted);
	ISteamUGC *steam_ugc = nullptr;

	void _on_item_downloaded(Ref<SteamworksCallbackData> p_callback);
	void _on_item_installed(Ref<SteamworksCallbackData> p_callback);

protected:
	static void _bind_methods();

public:
	TypedArray<HBSteamUGCItem> get_subscribed_items();
	void init_interface();
	bool is_valid() const;
	ISteamUGC *get_interface() const;
};

#endif // STEAM_UGC_H
