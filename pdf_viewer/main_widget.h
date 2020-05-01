#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>

#include <qwidget.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtimer.h>

#include <mupdf/fitz.h>
#include "document_view.h"
#include "document.h"
#include "input.h"
#include "book.h"
#include "config.h"
#include "pdf_renderer.h"
#include "input.h"
#include "pdf_view_opengl_widget.h"

extern float vertical_move_amount;
extern float horizontal_move_amount;
extern filesystem::path last_path_file_absolute_location;


class MainWidget : public QWidget {

private:
	fz_context* mupdf_context;
	sqlite3* db;
	DocumentManager* document_manager;
	CommandManager command_manager;
	ConfigManager* config_manager;
	PdfRenderer* pdf_renderer;
	InputHandler* input_handler;

	PdfViewOpenGLWidget* opengl_widget;
	PdfViewOpenGLWidget* helper_opengl_widget;

	const Command* current_pending_command = nullptr;

	DocumentView* main_document_view;
	DocumentView* helper_document_view;

	// current widget responsible for selecting an option (for example toc or bookmarks)
	unique_ptr<QWidget> current_widget;

	vector<DocumentViewState> history;
	int current_history_index;

	// last position when mouse was clicked in absolute document space
	float last_mouse_down_x;
	float last_mouse_down_y;
	bool is_selecting;
	wstring selected_text;

	Link* link_to_edit;

	optional<pair<optional<wstring>, Link>> pending_link;

	int main_window_width, main_window_height;

	QLineEdit* text_command_line_edit;
	QTextEdit* status_label;

	bool is_render_invalidated = false;
	bool is_ui_invalidated = false;

	bool should_delete_old_pages = false;

	QTimer garbage_collect_timer;
	QTime last_text_select_time = QTime::currentTime();

protected:

	void paintEvent(QPaintEvent* paint_event) override;
	void resizeEvent(QResizeEvent* resize_event) override;
	void mouseMoveEvent(QMouseEvent* mouse_event) override;
	void closeEvent(QCloseEvent* close_event) override;
	bool is_pending_link_source_filled();
	string get_status_string();
	void handle_escape();
	void keyPressEvent(QKeyEvent* kevent) override;
	void keyReleaseEvent(QKeyEvent* kevent) override;
	void validate_render();
	void validate_ui();
	void invalidate_render();
	void invalidate_ui();
	void handle_command_with_symbol(const Command* command, char symbol);
	void handle_command_with_file_name(const Command* command, wstring file_name);
	bool is_waiting_for_symbol();
	void key_event(bool released, QKeyEvent* kevent);
	void handle_left_click(float x, float y, bool down);
	void push_state();
	void next_state();
	void prev_state();
	void set_main_document_view_state(DocumentViewState new_view_state);
	void handle_click(int pos_x, int pos_y);
	void mouseReleaseEvent(QMouseEvent* mevent) override;
	void mousePressEvent(QMouseEvent* mevent) override;
	void wheelEvent(QWheelEvent* wevent) override;
	void show_textbar();
	void toggle_two_window_mode();
	void handle_command(const Command* command, int num_repeats);
	void handle_link();
	void handle_pending_text_command(wstring text);
	void toggle_fullscreen();

public:

	MainWidget(
		fz_context* mupdf_context,
		sqlite3* db,
		DocumentManager* document_manager,
		ConfigManager* config_manager,
		InputHandler* input_handler,
		bool* should_quit_ptr,
		QWidget* parent=nullptr
	);

	~MainWidget();

	void open_document(wstring path, optional<float> offset_x = {}, optional<float> offset_y = {});
};