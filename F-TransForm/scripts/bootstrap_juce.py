# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 Fazıl Irak
# Geliştirici: Fazıl Irak - 27.10.2026
# Dosyanın görevi: Sabit JUCE kaynak arşivini hash ile doğrulayıp Dependencies altına açmak.
from pathlib import Path
import hashlib,json,tarfile,tempfile,urllib.request
ROOT=Path(__file__).resolve().parents[1]
def sha(path):
    with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def bootstrap():
    lock=json.loads((ROOT/'source-lock.json').read_text(encoding='utf-8-sig'))
    target=ROOT/'Dependencies/JUCE'
    if (target/'CMakeLists.txt').exists():
        print('Using existing complete JUCE sources:',target);return
    with tempfile.TemporaryDirectory() as temporary:
        archive=Path(temporary)/'JUCE.tar.gz'
        urllib.request.urlretrieve(lock['juceArchiveUrl'],archive)
        if sha(archive)!=lock['juceArchiveSha256']:raise RuntimeError('JUCE archive SHA-256 mismatch')
        target.mkdir(parents=True,exist_ok=True)
        with tarfile.open(archive) as tar:
            prefix='JUCE-'+lock['juceCommit']+'/'
            members=[]
            for member in tar.getmembers():
                if member.name==prefix[:-1]:continue
                if not member.name.startswith(prefix):raise RuntimeError('Unexpected JUCE archive root')
                member.name=member.name[len(prefix):];members.append(member)
            tar.extractall(target,members=members,filter='data')
    print('Verified pinned JUCE source:',lock['juceCommit'])
if __name__=='__main__':bootstrap()

