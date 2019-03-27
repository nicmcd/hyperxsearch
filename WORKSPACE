load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

release = "1.8.1"
http_archive(
  name = "googletest",
  urls = ["https://github.com/google/googletest/archive/release-" + release + ".tar.gz"],
  strip_prefix = "googletest-release-" + release,
)

http_file(
  name = "cpplint_build",
  urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/cpplint.BUILD"],
)

release = "1.3.0"
http_archive(
  name = "cpplint",
  urls = ["https://github.com/cpplint/cpplint/archive/" + release + ".tar.gz"],
  strip_prefix = "cpplint-" + release,
  build_file = "@cpplint_build//file:downloaded",
)

http_file(
  name = "tclap_build",
  urls = ["https://raw.githubusercontent.com/nicmcd/pkgbuild/master/tclap.BUILD"],
)

version = "1.2.2"
http_archive(
  name = "tclap",
  urls = ["https://sourceforge.net/projects/tclap/files/tclap-" + version + ".tar.gz"],
  strip_prefix = "tclap-" + version,
  build_file = "@tclap_build//file:downloaded",
)

hash = "6b56ef3"
http_archive(
  name = "libprim",
  urls = ["https://github.com/nicmcd/libprim/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libprim-" + hash,
)

hash = "ad29c47"
http_archive(
  name = "libstrop",
  urls = ["https://github.com/nicmcd/libstrop/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libstrop-" + hash,
)

hash = "cbe9b3a"
http_archive(
  name = "libgrid",
  urls = ["https://github.com/nicmcd/libgrid/tarball/" + hash],
  type = "tar.gz",
  strip_prefix = "nicmcd-libgrid-" + hash,
)
