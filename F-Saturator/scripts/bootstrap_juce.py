# Fazıl Irak tarafından oluşturuldu: 23.10.2026
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Sabit JUCE arşivini SHA-256 ile doğrular; özgün dosyaları değiştirmeden açar."""
from pathlib import Path
import hashlib
import json
import sys
import tarfile
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.file_digest(path.open('rb'), 'sha256').hexdigest()


def verify_tree():
    manifest = json.loads((ROOT / 'vendor/JUCE-VERIFIED.json').read_text(encoding='utf-8'))
    lock = json.loads((ROOT / 'Research/dependencies.json').read_text(encoding='utf-8'))['juce']
    if manifest['commit'] != lock['commit'] or manifest['archive_sha256'] != lock['archive_sha256']:
        raise RuntimeError('JUCE dependency revision mismatch')
    base = ROOT / 'vendor/JUCE'
    actual = {p.relative_to(base).as_posix(): sha(p) for p in base.rglob('*') if p.is_file()}
    if actual != manifest['files']:
        raise RuntimeError('JUCE source differs from verified upstream archive')
    return manifest


def bootstrap(archive_path=None):
    if (ROOT / 'vendor/JUCE-VERIFIED.json').exists():
        verify_tree()
        return
    lock = json.loads((ROOT / 'Research/dependencies.json').read_text(encoding='utf-8'))['juce']
    destination = ROOT / 'vendor/JUCE'
    if destination.exists():
        raise RuntimeError('Unverified vendor/JUCE already exists; inspect it before replacing')
    with tempfile.TemporaryDirectory() as temporary:
        archive = Path(archive_path).resolve() if archive_path else Path(temporary) / 'juce.tar.gz'
        if not archive_path:
            urllib.request.urlretrieve(lock['archive_url'], archive)
        if sha(archive) != lock['archive_sha256']:
            raise RuntimeError('JUCE archive SHA-256 mismatch')
        destination.mkdir(parents=True)
        with tarfile.open(archive) as tar:
            prefix = 'JUCE-' + lock['commit'] + '/'
            members = []
            for member in tar.getmembers():
                if member.name == prefix[:-1]:
                    continue
                if not member.name.startswith(prefix):
                    raise RuntimeError('Unexpected archive root')
                member.name = member.name[len(prefix):]
                members.append(member)
            tar.extractall(destination, members=members, filter='data')
    files = {p.relative_to(destination).as_posix(): sha(p) for p in destination.rglob('*') if p.is_file()}
    marker = {'commit': lock['commit'], 'archive_sha256': lock['archive_sha256'], 'files': files}
    (ROOT / 'vendor/JUCE-VERIFIED.json').write_text(json.dumps(marker, indent=2) + '\n', encoding='utf-8')
    print('Verified JUCE source files:', len(files))


if __name__ == '__main__':
    bootstrap(sys.argv[1] if len(sys.argv) > 1 else None)
