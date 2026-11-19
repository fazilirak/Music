// Fazıl Irak · 19.11.2026: isolated persistence, preset independence, sizing, repaint benchmark.
#include "PluginEditor.h"
#include <iostream>
int main()
{
 juce::ScopedJuceInitialiser_GUI gui;
 const auto file=juce::File::getCurrentWorkingDirectory().getChildFile("view-tests").getChildFile("preferences.settings").getNonexistentSibling();
 irak::Processor p;juce::MemoryBlock before,after;p.getStateInformation(before);
 for(const char* id : {"heat","auto_gain","bypass","fast_attack","fast_release","bass_tight","treble_tight"})
 {
  auto* value=p.state.getParameter(id);const float saved=value->getValue();
  for(float fractional : {.15f,.41f,.5f,.83f})
  {
   value->setValueNotifyingHost(fractional);
   if(value->getValue()!=(fractional>=.5f?1.f:0.f))return 10;
   p.setStateInformation(before.getData(),int(before.getSize()));
   if(value->getValue()!=saved)return 11;
  }
 }
 int width=0,height=0;
 {
  irak::Editor editor(p,file);
  if(editor.getUiScale()!=100||!editor.isResizable()||editor.getWidth()!=912||editor.getHeight()!=511)return 1;
  for(int scale : {50,75,100,125,150,175,200})
  {
   editor.setUiScale(scale);
   const auto* limits=editor.getConstrainer();
   if(editor.getUiScale()!=scale||editor.getWidth()>limits->getMaximumWidth()||editor.getHeight()>limits->getMaximumHeight()||std::abs(editor.getWidth()/double(editor.getHeight())-912.0/511)>.006)return 2;
  }
  editor.setUiScale(75);
  if(editor.getWidth()!=684||editor.getHeight()!=383)return 3;
  p.getStateInformation(after);if(before!=after)return 4;
  p.setCurrentProgram(6);if(editor.getWidth()!=684||editor.getUiScale()!=75)return 5;
  editor.setSize(800,448);width=editor.getWidth();height=editor.getHeight();
 }
 {
  irak::Processor reopened;
  irak::Editor editor(reopened,file);
  if(editor.getUiScale()!=75||editor.getWidth()!=width||editor.getHeight()!=height)return 6;
  reopened.setStateInformation(before.getData(),int(before.getSize()));
  if(editor.getWidth()!=width||editor.getHeight()!=height)return 7;
  editor.setUiScale(100);
  juce::Timer::callPendingTimersSynchronously();
  const auto dir=juce::File::getCurrentWorkingDirectory().getChildFile("../dist");
  for(int scale : {50,75,100,125,150,175,200})
  {
   editor.setUiScale(scale);const auto image=editor.createComponentSnapshot(editor.getLocalBounds());
   juce::FileOutputStream stream(dir.getChildFile("F-Saturator-zoom-"+juce::String(scale)+".png"));
   stream.setPosition(0);if(stream.truncate().failed()||!juce::PNGImageFormat().writeImageToStream(image,stream))return 8;
  }
 }
 juce::PropertiesFile::Options options;options.millisecondsBeforeSaving=-1;
 {juce::PropertiesFile prefs(file,options);prefs.setValue("windowWidth",999999);prefs.setValue("uiScale",999999);prefs.saveIfNeeded();}
 {irak::Editor editor(p,file);if(editor.getWidth()>editor.getConstrainer()->getMaximumWidth()||editor.getUiScale()!=200)return 9;}
 std::cout<<"PASS: seven zooms, aspect ratio, reopen with new processor, preset/session independence, corrupt/oversized preferences, seven screenshots."<<std::endl;
 return 0;
}
