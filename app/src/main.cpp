#include <cmark.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Post {
	std::string slug;
	std::string title;
	std::string html;
};

// Get content directory (relative to executable)
fs::path get_content_dir() {
	// Get executable path
	fs::path exe_path = fs::current_path();

	// Since exe is at: C:/project-coding/cpp/202608/blog/bin/clang-cl/blog-parser.exe
	// Content is at: C:/project-coding/cpp/202608/blog/content

	// Go up 3 levels: bin/clang-cl/ -> blog/
	fs::path content_dir = exe_path
							   .parent_path() // bin/clang-cl/
							   .parent_path() // bin/
						   / "content";

	return content_dir;
}

std::string read_file(const fs::path &path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "❌ Cannot open: " << path << std::endl;
		return "";
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void write_file(const fs::path &path, const std::string &content) {
	fs::create_directories(path.parent_path());
	std::ofstream file(path);
	file << content;
}

std::string md_to_html(const std::string &md) {
	char *html = cmark_markdown_to_html(md.c_str(), md.size(), CMARK_OPT_DEFAULT);
	std::string result(html);
	free(html);
	return result;
}

std::string layout(const std::string &content) {
	return "<!DOCTYPE html>\n"
		   "<html>\n"
		   "<head><meta charset='utf-8'><title>My Blog</title></head>\n"
		   "<body>\n"
		   "<nav><a href='/'>Home</a> | <a href='/about.html'>About</a></nav>\n" +
		   content +
		   "</body>\n"
		   "</html>";
}

std::string extract_title(const std::string &md) {
	size_t pos = md.find('#');
	if (pos != std::string::npos) {
		size_t end = md.find('\n', pos);
		std::string title = md.substr(pos + 1, end - pos - 1);
		while (!title.empty() && title.front() == ' ')
			title.erase(0, 1);
		while (!title.empty() && title.back() == ' ')
			title.pop_back();
		return title;
	}
	return "Untitled";
}

int main() {
	try {
		// Determine paths
		fs::path content_dir = get_content_dir();
		fs::path posts_dir = content_dir / "posts";
		fs::path output_dir = fs::current_path().parent_path().parent_path() / "html";

		std::cout << "📁 Content: " << content_dir << std::endl;
		std::cout << "📁 Output:  " << output_dir << std::endl;

		if (!fs::exists(content_dir)) {
			std::cerr << "❌ Content directory not found: " << content_dir << std::endl;
			return 1;
		}

		fs::create_directories(output_dir);

		// 1. Generate posts
		std::vector<Post> posts;
		if (fs::exists(posts_dir) && fs::is_directory(posts_dir)) {
			for (const auto &entry : fs::directory_iterator(posts_dir)) {
				if (entry.path().extension() == ".md") {
					std::string md = read_file(entry.path());
					Post post;
					post.slug = entry.path().stem().string();
					post.title = extract_title(md);
					post.html = md_to_html(md);
					posts.push_back(post);

					std::string page = layout("<h1>" + post.title + "</h1>" + post.html);
					write_file(output_dir / "posts" / (post.slug + ".html"), page);
					std::cout << "✅ Generated: " << post.slug << std::endl;
				}
			}
		} else {
			std::cout << "⚠️  No posts directory found at: " << posts_dir << std::endl;
		}

		// 2. Generate homepage
		fs::path index_md = content_dir / "index.md";
		std::string home_content = fs::exists(index_md) ? read_file(index_md) : "# My Blog\nWelcome!";

		std::string post_list = "<ul>";
		for (const auto &post : posts) {
			post_list += "<li><a href='/posts/" + post.slug + ".html'>" + post.title + "</a></li>";
		}
		post_list += "</ul>";

		std::string home_html = layout(md_to_html(home_content) + post_list);
		write_file(output_dir / "index.html", home_html);

		// 3. Generate about page
		fs::path about_md = content_dir / "about.md";
		if (fs::exists(about_md)) {
			std::string about_md_content = read_file(about_md);
			std::string about_html = layout("<h1>About</h1>" + md_to_html(about_md_content));
			write_file(output_dir / "about.html", about_html);
		}

		std::cout << "\n✅ Generated " << posts.size() << " posts" << std::endl;
		std::cout << "📂 Output in: " << output_dir << std::endl;

	} catch (const std::exception &e) {
		std::cerr << "❌ Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
