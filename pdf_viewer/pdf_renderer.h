#pragma once

#include <vector>
#include <string>
#include <mupdf/fitz.h>
#include <mutex>
#include <variant>
#include <unordered_map>
#include <map>
#include <optional>
#include <SDL.h>
#include <iostream>
#include <gl/glew.h>
#include <Windows.h>

#include "book.h"

extern const int max_pending_requests;
extern const unsigned int cache_invalid_milies;

struct RenderRequest {
	wstring path;
	int page;
	float zoom_level;
};

struct SearchRequest {
	wstring path;
	int start_page;
	wstring search_term;
	vector<SearchResult>* search_results;
	mutex* search_results_mutex;
	float* percent_done;
	bool* is_searching;
};

struct RenderResponse {
	RenderRequest request;
	unsigned int last_access_time;
	int thread;
	fz_pixmap* pixmap;
	GLuint texture;
};

bool operator==(const RenderRequest& lhs, const RenderRequest& rhs);

class PdfRenderer {
	// A pointer to the mupdf context to clone.
	// Since the context should only be used from the thread that initialized it,
	// we can not simply clone the context in the initializer because the initializer
	// is called from the main thread. Instead, we just save a pointer to the context
	// in the initializer and then clone the context when run() is called in the worker
	//thread.
	fz_context* context_to_clone;

	////cloned mupdf context to use in the worker thread
	//fz_context* mupdf_context;

	vector<vector<fz_pixmap*>> pixmaps_to_drop;
	map<pair<int, wstring>, fz_document*> opened_documents;
	//vector<variant<RenderRequest, SearchRequest>> pending_requests;
	vector<RenderRequest> pending_render_requests;
	optional<SearchRequest> pending_search_request;
	vector<RenderResponse> cached_responses;
	vector<thread> worker_threads;
	thread search_thread;
	//mutex for pending render requests
	mutex pending_requests_mutex;
	mutex search_request_mutex;
	mutex cached_response_mutex;
	//mutex pixmap_drop_mutex;
	vector<mutex> pixmap_drop_mutex;

	// a pointer to a boolean variable indicating whether render is invalidated,
	// this is so that we can initiate a rerender when worker thread has rendered
	// a new page. (todo: this is very hacky, should be handled more elegantly)
	bool* invalidate_pointer;
	bool* should_quit_pointer;

	int num_threads;

public:

	PdfRenderer(int num_threads, bool* should_quit_pointer, fz_context* context_to_clone);

	fz_context* init_context();

	void start_threads();
	void join_threads();

	void set_invalidate_pointer(bool* inv_p);

	fz_document* get_document_with_path(int thread_index, fz_context* mupdf_context, wstring path);

	//should only be called from the main thread
	void add_request(wstring document_path, int page, float zoom_level);
	void add_request(wstring document_path, int page, wstring term, vector<SearchResult>* out, float* percent_done, bool* is_searching, mutex* mut);

	//should only be called from the main thread
	GLuint find_rendered_page(wstring path, int page, float zoom_level, int* page_width, int* page_height);

	GLuint try_closest_rendered_page(wstring doc_path, int page, float zoom_level, int* page_width, int* page_height);

	void delete_old_pages();

	void delete_old_pixmaps(int thread_index, fz_context* mupdf_context);

	void run(int thread_index);
	void run_search(int thread_index);

	~PdfRenderer();

};