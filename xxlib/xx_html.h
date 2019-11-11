#pragma once
#include "xx_bbuffer.h"

// 一堆辅助拼接 html 输出的类

namespace xx::Html {
	struct Base {
		virtual ~Base() {}
		virtual void ToHtml(std::string& s) const noexcept = 0;

		std::vector<std::shared_ptr<Base>> childs;

		template<typename T>
		std::shared_ptr<T> Add(std::shared_ptr<T>&& item) {
			return xx::As<T>(childs.emplace_back(std::move(item)));
		}
	};
}

namespace xx {
	// 适配 Base
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_base_of_v<xx::Html::Base, T>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			in.ToHtml(s);
		}
	};

	// 适配 std::shared_ptr<Base>
	template<typename T>
	struct SFuncs<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<xx::Html::Base, T>>> {
		static inline void WriteTo(std::string& s, std::shared_ptr<T> const& in) noexcept {
			if (in) {
				in->ToHtml(s);
			}
			else {
				s.append("");
			}
		}
	};
}

namespace xx::Html {

	struct Document : Base {
		Document() = default;
		Document(Document const&) = delete;
		Document(Document&&) = default;
		Document& operator=(Document const&) = delete;
		Document& operator=(Document&&) = default;

		// todo: css? title ??

		inline static std::shared_ptr<Document> Create() {
			return xx::Make<Document>();
		}

		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<html>", childs, "</html>");
		}

		// 直接拼接辅助函数
		inline static void AppendHead(std::string& s) {
			xx::Append(s, "<html>");
		}
		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "</html>");
		}
	};

	struct Body : Base {
		Body() = default;
		Body(Body const&) = delete;
		Body(Body&&) = default;
		Body& operator=(Body const&) = delete;
		Body& operator=(Body&&) = default;

		inline static std::shared_ptr<Body> Create() {
			return xx::Make<Body>();
		}

		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<body>", childs, "</body>");
		}

		// 直接拼接辅助函数
		inline static void AppendHead(std::string& s) {
			xx::Append(s, "<body>");
		}
		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "</body>");
		}
	};

	struct Form : Base {
		Form(Form const&) = delete;
		Form(Form&&) = default;
		Form& operator=(Form const&) = delete;
		Form& operator=(Form&&) = default;

		Form(std::string&& action = "")
			: action(std::move(action)) {
		}

		inline static std::shared_ptr<Form> Create(std::string&& action = "") {
			return xx::Make<Form>(std::move(action));
		}

		std::string action;

		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<form method=\"post\" action=\"", action, "\">", childs, "<input type=\"submit\" value=\"Submit\" /></form>");
		}

		// 直接拼接辅助函数
		inline static void AppendHead(std::string& s, std::string&& action = "") {
			xx::Append(s, "<form method=\"post\" action=\"", action, "\">");
		}
		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "<input type=\"submit\" value=\"Submit\" /></form>");
		}
	};

	struct Input : Base {
		Input() = default;
		Input(Input const&) = delete;
		Input(Input&&) = default;
		Input& operator=(Input const&) = delete;
		Input& operator=(Input&&) = default;

		inline static std::shared_ptr<Input> Create(std::string&& title, std::string&& name = "", std::string&& value = "", std::string&& type = "") {
			auto&& rtv = xx::Make<Input>();
			rtv->title = std::move(title);
			rtv->name = std::move(name);
			rtv->value = std::move(value);
			rtv->type = std::move(type);
			return rtv;
		}

		std::string title;
		std::string name;
		std::string value;
		std::string type;
		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<p>", (title.size() ? title : name), ":<input type=\"", (type.size() ? type : std::string("text")), "\" name=\"", (name.size() ? name : title), "\" value=\"", value, "\" /></p>");
		}

		// 直接拼接辅助函数
		inline static void Append(std::string& s, std::string&& title, std::string&& name = "", std::string&& value = "", std::string&& type = "") {
			xx::Append(s, "<p>", (title.size() ? title : name), ":<input type=\"", (type.size() ? type : std::string("text")), "\" name=\"", (name.size() ? name : title), "\" value=\"", value, "\" /></p>");
		}
	};

	struct HyperLink : Base {
		HyperLink() = default;
		HyperLink(HyperLink const&) = delete;
		HyperLink(HyperLink&&) = default;
		HyperLink& operator=(HyperLink const&) = delete;
		HyperLink& operator=(HyperLink&&) = default;

		template<typename ...Args>
		inline static std::shared_ptr<HyperLink> Create(std::string&& content, Args const& ... hrefs) {
			auto&& rtv = xx::Make<HyperLink>();
			xx::Append(rtv->href, hrefs...);
			rtv->content = std::move(content);
			return rtv;
		}

		template<typename ...Args>
		inline void SetHref(Args const& ... hrefs) {
			href.clear();
			xx::Append(href, hrefs...);
		}

		template<typename ...Args>
		inline void SetContent(Args const& ... contents) {
			content.clear();
			xx::Append(content, contents...);
		}

		std::string href;
		std::string content;
		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<a href=\"", href, "\">", content, childs, "</a>");
		}

		// 直接拼接辅助函数
		template<typename ...Args>
		inline static void AppendHead(std::string& s, Args const& ... hrefs) {
			xx::Append(s, "<a href=\"", hrefs..., "\">");
		}
		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "</a>");
		}

		template<typename ...Args>
		inline static void Append(std::string& s, std::string&& content, Args const& ... hrefs) {
			xx::Append(s, "<a href=\"", hrefs..., "\">", content, "</a>");
		}
	};

	struct Paragrapth : Base {
		Paragrapth() = default;
		Paragrapth(Paragrapth const&) = delete;
		Paragrapth(Paragrapth&&) = default;
		Paragrapth& operator=(Paragrapth const&) = delete;
		Paragrapth& operator=(Paragrapth&&) = default;

		template<typename ...Args>
		inline static std::shared_ptr<Paragrapth> Create(Args const& ... contents) {
			auto&& rtv = xx::Make<Paragrapth>();
			xx::Append(rtv->content, contents...);
			return rtv;
		}

		template<typename ...Args>
		inline void SetContent(Args const& ... contents) {
			content.clear();
			xx::Append(content, contents...);
		}

		std::string content;
		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, "<p>", content, childs, "</p>");
		}

		// 直接拼接辅助函数
		inline static void AppendHead(std::string& s) {
			xx::Append(s, "<p>");
		}
		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "</p>");
		}
	};

	// 这个就是直接字符串本身, 前后不加料
	struct Literal : Base {
		Literal() = default;
		Literal(Literal const&) = delete;
		Literal(Literal&&) = default;
		Literal& operator=(Literal const&) = delete;
		Literal& operator=(Literal&&) = default;

		template<typename ...Args>
		inline static std::shared_ptr<Literal> Create(Args const& ... contents) {
			auto&& rtv = xx::Make<Literal>();
			xx::Append(rtv->content, contents...);
			return rtv;
		}

		template<typename ...Args>
		inline void SetContent(Args const& ... contents) {
			content.clear();
			xx::Append(content, contents...);
		}

		std::string content;
		inline virtual void ToHtml(std::string& s) const noexcept override {
			xx::Append(s, content, childs);
		}
	};

	struct Table : Base {
		Table() = default;
		Table(Table const&) = delete;
		Table(Table&&) = default;
		Table& operator=(Table const&) = delete;
		Table& operator=(Table&&) = default;

		inline static std::shared_ptr<Table> Create(int&& numColumns
			, std::function<void(int const& columnIndex, std::string & s)> titleFiller
			, std::function<bool(int const& rowIndex, int const& columnIndex, std::string & s)> columnFiller) {
			auto&& rtv = xx::Make<Table>();
			rtv->numColumns = std::move(numColumns);
			rtv->titleFiller = std::move(titleFiller);
			rtv->columnFiller = std::move(columnFiller);
			return rtv;
		}

		// 列数( 需设定 )
		int numColumns = 0;

		std::function<void(int const& columnIndex, std::string & s)> titleFiller;
		std::function<bool(int const& rowIndex, int const& columnIndex, std::string & s)> columnFiller;

		inline virtual void ToHtml(std::string& s) const noexcept override {
			if (!numColumns) return;
			xx::Append(s, "<table border=\"1\">");
			if (titleFiller) {
				xx::Append(s, "<thead><tr>");
				for (int i = 0; i < numColumns; ++i) {
					xx::Append(s, "<th>");
					titleFiller(i, s);
					xx::Append(s, "</th>");
				}
				xx::Append(s, "</tr></thead>");
			}
			if (columnFiller) {
				bool tryAgain = true;
				int rowIndex = 0;
				xx::Append(s, "<tbody>");
				do {
					xx::Append(s, "<tr>");
					for (int i = 0; i < numColumns; ++i) {
						xx::Append(s, "<td>");
						tryAgain = columnFiller(rowIndex, i, s);
						xx::Append(s, "</td>");
					}
					++rowIndex;
				} while (tryAgain);
				xx::Append(s, "</tbody>");
			}
			xx::Append(s, "</table>");
		}


		// 下面是直接追加拼接模式辅助函数

		template<typename ...Args>
		inline static void AppendHead(std::string& s, Args const& ... titles) {
			xx::Append(s, "<table border=\"1\">");
			xx::Append(s, "<thead><tr>");
			std::initializer_list<int> n{ ((AppendHeadCore(s, titles)), 0)... };
			(void)(n);
			xx::Append(s, "</tr></thead><tbody>");
		}

		template<typename ...Args>
		inline static void AppendRow(std::string& s, Args const& ... columns) {
			xx::Append(s, "<tr>");
			std::initializer_list<int> n{ ((AppendRowCore(s, columns)), 0)... };
			(void)(n);
			xx::Append(s, "</tr>");
		}

		inline static void AppendFoot(std::string& s) {
			xx::Append(s, "</tbody></table>");
		}

	protected:
		template<typename T>
		inline static void AppendHeadCore(std::string& s, T const& v) {
			xx::Append(s, "<th>", v, "</th>");
		}

		template<typename T>
		inline static void AppendRowCore(std::string& s, T const& v) {
			xx::Append(s, "<td>", v, "</td>");
		}
	};
}
