# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Gömülü iki fontun özgün kaynak koleksiyonunu doğrulanmış arşivden açar."""
from pathlib import Path
import hashlib
import json
import sys
import tarfile
import tempfile
import urllib.request
from bootstrap_juce import ROOT, sha

URL = 'https://codeload.github.com/ArtifexSoftware/urw-base35-fonts/tar.gz/refs/tags/20200910'
DIGEST = 'e0d9b7f11885fdfdc4987f06b2aa0565ad2a4af52b22e5ebf79e1a98abd0ae2f'


def verify():
    marker = json.loads((ROOT / 'vendor/URW-VERIFIED.json').read_text(encoding='utf-8'))
    if marker['archive_sha256'] != DIGEST:
        raise RuntimeError('Unexpected URW source revision')
    base = ROOT / 'vendor/urw-base35-fonts'
    if marker['files'] != {p.relative_to(base).as_posix(): sha(p) for p in base.rglob('*') if p.is_file()}:
        raise RuntimeError('URW sources differ from the verified archive')
    for local, upstream in [('Book.otf', 'URWGothic-Book.otf'), ('Demi.otf', 'URWGothic-Demi.otf')]:
        if sha(ROOT / 'Resources/fonts' / local) != sha(base / 'fonts' / upstream):
            raise RuntimeError('Embedded font differs from the declared source')


def bootstrap(path=None):
    if (ROOT / 'vendor/URW-VERIFIED.json').exists():
        verify()
        return
    base = ROOT / 'vendor/urw-base35-fonts'
    if base.exists():
        raise RuntimeError('Unverified URW directory already exists')
    with tempfile.TemporaryDirectory() as temporary:
        archive = Path(path).resolve() if path else Path(temporary) / 'urw.tar.gz'
        if not path:
            urllib.request.urlretrieve(URL, archive)
        if sha(archive) != DIGEST:
            raise RuntimeError('URW archive SHA-256 mismatch')
        base.mkdir(parents=True)
        with tarfile.open(archive) as tar:
            members = []
            prefix = 'urw-base35-fonts-20200910/'
            for member in tar.getmembers():
                if member.name == prefix[:-1]:
                    continue
                if not member.name.startswith(prefix):
                    raise RuntimeError('Unexpected URW archive root')
                member.name = member.name[len(prefix):]
                members.append(member)
            tar.extractall(base, members=members, filter='data')
    marker = {'tag': '20200910', 'archive_sha256': DIGEST,
              'files': {p.relative_to(base).as_posix(): sha(p) for p in base.rglob('*') if p.is_file()}}
    (ROOT / 'vendor/URW-VERIFIED.json').write_text(json.dumps(marker, indent=2) + '\n', encoding='utf-8')
    verify()
    print('Verified URW sources and embedded fonts:', len(marker['files']))


if __name__ == '__main__':
    bootstrap(sys.argv[1] if len(sys.argv) > 1 else None)
