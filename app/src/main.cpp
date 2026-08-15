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

std::string md_to_html(const std::string &md) {
	char *html = cmark_markdown_to_html(md.c_str(), md.size(), CMARK_OPT_DEFAULT);
	std::string result(html);
	free(html);
	return result;
}

std::string read_file(const std::string &path) {
	std::ifstream file(path);
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void write_file(const std::string &path, const std::string &content) {
	fs::create_directories(fs::path(path).parent_path());
	std::ofstream file(path);
	file << content;
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
	// First line starting with # is the title
	size_t pos = md.find('#');
	if (pos != std::string::npos) {
		size_t end = md.find('\n', pos);
		std::string title = md.substr(pos + 1, end - pos - 1);
		// Trim whitespace
		while (!title.empty() && title.front() == ' ')
			title.erase(0, 1);
		while (!title.empty() && title.back() == ' ')
			title.pop_back();
		return title;
	}
	return "Untitled";
}

int main() {
	// Create output directory
	fs::create_directory("output");

	// 1. Generate posts
	std::vector<Post> posts;
	for (const auto &entry : fs::directory_iterator("content/posts")) {
		if (entry.path().extension() == ".md") {
			std::string md = read_file(entry.path().string());
			Post post;
			post.slug = entry.path().stem().string();
			post.title = extract_title(md);
			post.html = md_to_html(md);
			posts.push_back(post);

			// Write individual post page
			std::string page = layout("<h1>" + post.title + "</h1>" + post.html);
			write_file("output/posts/" + post.slug + ".html", page);
		}
	}

	// 2. Generate homepage (list all posts)
	std::string home_content = read_file("content/index.md");
	std::string post_list = "<ul>";
	for (const auto &post : posts) {
		post_list += "<li><a href='/posts/" + post.slug + ".html'>" + post.title + "</a></li>";
	}
	post_list += "</ul>";

	std::string home_html = layout(md_to_html(home_content) + post_list);
	write_file("output/index.html", home_html);

	// 3. Generate about page
	std::string about_md = read_file("content/about.md");
	std::string about_html = layout("<h1>About</h1>" + md_to_html(about_md));
	write_file("output/about.html", about_html);

	std::cout << "✅ Generated " << posts.size() << " posts" << std::endl;
	return 0;
}
