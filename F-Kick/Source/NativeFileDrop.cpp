// Geliştirici: Fazıl Irak - Son düzenleme: 27.10.2026
// Dosyanın görevi: Windows dosya bırakma olaylarını asenkron sample yükleyicisine taşımak.
// Copyright (c) 2026 Fazıl Irak - Özgün F-Kick uygulama kodu.
// SPDX-License-Identifier: AGPL-3.0-or-later
// Fazıl Irak · CF_HDROP verisini Unicode olarak alır; yükleme GUI/worker yolunda kalır.
#include "NativeFileDrop.h"
#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
struct NativeFileDrop::Impl final : IDropTarget {
    explicit Impl(NativeFileDrop& o):owner(o){}
    NativeFileDrop& owner;juce::Component::SafePointer<juce::Component> editor;
    HWND window=nullptr;bool ole=false,installed=false;LONG refs=1;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id,void** out) override {if(!out)return E_POINTER;*out=nullptr;if(id==IID_IUnknown||id==IID_IDropTarget){*out=static_cast<IDropTarget*>(this);AddRef();return S_OK;}return E_NOINTERFACE;}
    ULONG STDMETHODCALLTYPE AddRef() override {return (ULONG)InterlockedIncrement(&refs);}
    ULONG STDMETHODCALLTYPE Release() override {auto n=InterlockedDecrement(&refs);if(n==0)delete this;return (ULONG)n;}
    juce::StringArray files;
    static juce::StringArray read(IDataObject* data){
        juce::StringArray result;if(!data)return result;FORMATETC format{CF_HDROP,nullptr,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};STGMEDIUM medium{};
        if(FAILED(data->GetData(&format,&medium)))return result;
        if(medium.tymed==TYMED_HGLOBAL&&medium.hGlobal){auto h=(HDROP)medium.hGlobal;const UINT n=DragQueryFileW(h,0xffffffff,nullptr,0);
            if(n==1){const auto size=DragQueryFileW(h,0,nullptr,0);if(size>0&&size<32768){std::vector<wchar_t> name(size+1);DragQueryFileW(h,0,name.data(),size+1);result.add(juce::String(name.data()));}}}
        ReleaseStgMedium(&medium);return result;
    }
    juce::Point<int> local(POINTL p) const {if(!editor)return {};POINT point{p.x,p.y};ScreenToClient(window,&point);auto* peer=editor->getPeer();const float scale=peer?float(peer->getPlatformScaleFactor()):1.f;const juce::Point<int> logical{juce::roundToInt(point.x/scale),juce::roundToInt(point.y/scale)};return peer?editor->getLocalPoint(&peer->getComponent(),logical):logical;}
    bool interested() const{return editor&&owner.accepts&&owner.accepts(files);}
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* data,DWORD,POINTL p,DWORD* effect) override {files=read(data);return DragOver(0,p,effect);}
    HRESULT STDMETHODCALLTYPE DragOver(DWORD,POINTL p,DWORD* effect) override {const bool yes=interested();if(effect)*effect=yes?DROPEFFECT_COPY:DROPEFFECT_NONE;if(owner.hover)owner.hover(files,local(p),yes);return S_OK;}
    HRESULT STDMETHODCALLTYPE DragLeave() override {if(owner.hover)owner.hover(files,{},false);files.clear();return S_OK;}
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* data,DWORD,POINTL p,DWORD* effect) override {files=read(data);const bool yes=interested();if(effect)*effect=yes?DROPEFFECT_COPY:DROPEFFECT_NONE;if(owner.hover)owner.hover(files,{},false);if(yes&&owner.drop)owner.drop(files,local(p));files.clear();return S_OK;}
    void detach(){if(installed&&IsWindow(window))RevokeDragDrop(window);installed=false;window=nullptr;editor=nullptr;if(ole){OleUninitialize();ole=false;}}
    void attach(juce::Component& c){auto* peer=c.getPeer();if(!peer)return;auto hwnd=(HWND)peer->getNativeHandle();if(hwnd==window)return;detach();
        // Yalnız JUCE'nin bu editör için oluşturduğu peer; host ebeveyn HWND'ye dokunma.
        if(!hwnd||!IsWindow(hwnd))return;DWORD pid=0;GetWindowThreadProcessId(hwnd,&pid);if(pid!=GetCurrentProcessId())return;
        const auto hr=OleInitialize(nullptr);if(FAILED(hr))return;ole=true;window=hwnd;editor=&c;
        RevokeDragDrop(window);installed=SUCCEEDED(RegisterDragDrop(window,this));
    }
};
NativeFileDrop::NativeFileDrop():impl(new Impl(*this)){}
NativeFileDrop::~NativeFileDrop(){impl->detach();}
void NativeFileDrop::attach(juce::Component& c){impl->attach(c);}
bool NativeFileDrop::registered() const noexcept{return impl->installed;}
// Test gerçek OLE IDataObject/CF_HDROP ayrıştırmasını ve kayıtlı hedefin Drop metodunu çalıştırır.
bool NativeFileDrop::testNativeTransfer(const juce::StringArray& files,juce::Point<int> pos){
    if(!registered()||files.size()!=1)return false;
    struct Data final : IDataObject {
        juce::String name;explicit Data(juce::String n):name(std::move(n)){}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void**) override{return E_NOINTERFACE;}ULONG STDMETHODCALLTYPE AddRef() override{return 1;}ULONG STDMETHODCALLTYPE Release() override{return 1;}
        HRESULT STDMETHODCALLTYPE GetData(FORMATETC* f,STGMEDIUM* m) override {if(f->cfFormat!=CF_HDROP)return DV_E_FORMATETC;auto text=name.toWideCharPointer();const auto bytes=(name.length()+2)*sizeof(wchar_t);auto h=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(DROPFILES)+bytes);if(!h)return E_OUTOFMEMORY;auto* p=(DROPFILES*)GlobalLock(h);p->pFiles=sizeof(DROPFILES);p->fWide=TRUE;std::memcpy((char*)p+sizeof(DROPFILES),text,(name.length()+1)*sizeof(wchar_t));GlobalUnlock(h);m->tymed=TYMED_HGLOBAL;m->hGlobal=h;m->pUnkForRelease=nullptr;return S_OK;}
        HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC*,STGMEDIUM*) override{return E_NOTIMPL;}HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC*) override{return S_OK;}HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC*,FORMATETC*) override{return E_NOTIMPL;}HRESULT STDMETHODCALLTYPE SetData(FORMATETC*,STGMEDIUM*,BOOL) override{return E_NOTIMPL;}HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD,IEnumFORMATETC**) override{return E_NOTIMPL;}HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*,DWORD,IAdviseSink*,DWORD*) override{return OLE_E_ADVISENOTSUPPORTED;}HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override{return OLE_E_ADVISENOTSUPPORTED;}HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA**) override{return OLE_E_ADVISENOTSUPPORTED;}
    } data(files[0]);
    auto point=impl->editor->localPointToGlobal(pos);POINTL p{point.x,point.y};DWORD effect=DROPEFFECT_COPY;impl->DragEnter(&data,0,p,&effect);impl->Drop(&data,0,p,&effect);return effect==DROPEFFECT_COPY;
}
#else
struct NativeFileDrop::Impl {};
NativeFileDrop::NativeFileDrop():impl(new Impl){}NativeFileDrop::~NativeFileDrop()=default;
void NativeFileDrop::attach(juce::Component&){}bool NativeFileDrop::registered()const noexcept{return false;}
bool NativeFileDrop::testNativeTransfer(const juce::StringArray&,juce::Point<int>){return false;}
#endif
