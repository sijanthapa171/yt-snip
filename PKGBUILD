# Maintainer: Your Name <your.email@domain.com>
pkgname=yt-snip
pkgver=1.0.0
pkgrel=1
pkgdesc="Fast YouTube video/audio downloader and trimmer - a yt-dlp wrapper"
arch=('x86_64')
url="https://github.com/melqtx/yt-snip"
license=('MIT')
depends=('yt-dlp' 'ffmpeg')
makedepends=('cmake' 'gcc')
source=("git+${url}.git")
sha256sums=('SKIP')

build() {
    cd "$srcdir/$pkgname"
    mkdir -p build
    cd build
    cmake ..
    make
}

package() {
    cd "$srcdir/$pkgname/build"
    make DESTDIR="$pkgdir/" install
    install -Dm644 ../LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
