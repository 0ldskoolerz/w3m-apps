# Maintainer: 0ldskoolerz <0ldskoolerz@users.noreply.github.com>
pkgname=w3m-apps
pkgver=0.1.0
pkgrel=1
pkgdesc="Suite de apps Win 3.x para W3M: explorador, terminal, tareas, calculadora y notepad (Xlib + busybox)"
arch=('x86_64' 'i686' 'aarch64')
url="https://github.com/0ldskoolerz/w3m-apps"
license=('MIT')
depends=('libx11' 'busybox')
makedepends=('git' 'pkgconf')
source=("$pkgname::git+$url.git#tag=v$pkgver")
md5sums=('SKIP')

build() {
    cd "$pkgname"
    make
}

package() {
    cd "$pkgname"
    for bin in w3m-fm w3m-term w3m-task w3m-calc w3m-notepad; do
        install -Dm755 "$bin" "$pkgdir/usr/bin/$bin"
    done
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
}
