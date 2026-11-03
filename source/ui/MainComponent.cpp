#include "MainComponent.h"

#include "ui/SettingsDialog.h"

namespace rig
{
    MainComponent::MainComponent (juce::AudioDeviceManager& dm, AudioEngine& e, RigModel& m)
        : deviceManager (dm),
          engine (e),
          model (m),
          topBar (RigModel::numPatches),
          chainView (m),
          blockEditor (m, e)
    {
        topBar.setPatchNumber (model.getPatchNumber());
        topBar.onPatchChanged = [this] (int number)
        {
            model.loadPatch (number);

            // 알림 없이 값만 바꾼다 (알림이 가면 새 패치 파일을 다시 저장하게 됨)
            inputKnob.setValueDb (model.getInputGainDb());
            outputKnob.setValueDb (model.getOutputGainDb());
        };
        topBar.onSettingsClicked = [this] { openSettings(); };

        inputKnob.setValueDb (model.getInputGainDb());
        outputKnob.setValueDb (model.getOutputGainDb());
        inputKnob.onValueChanged  = [this] (float db) { model.setInputGainDb (db); };
        outputKnob.onValueChanged = [this] (float db) { model.setOutputGainDb (db); };

        addAndMakeVisible (topBar);
        addAndMakeVisible (chainView);
        addAndMakeVisible (blockEditor);
        addAndMakeVisible (statusBar);
        addAndMakeVisible (inputKnob);
        addAndMakeVisible (outputKnob);

        startTimerHz (30);
        setSize (1024, 720);
    }

    void MainComponent::openSettings()
    {
        juce::Component::SafePointer<MainComponent> safeThis (this);

        SettingsDialog::show (deviceManager, this, [safeThis] (bool saved)
        {
            if (safeThis != nullptr && saved)
                safeThis->model.deviceChanged();
        });
    }

    void MainComponent::timerCallback()
    {
        statusBar.update (engine.takeInputPeak(), deviceManager.getCpuUsage(), engine.takeOutputPeak());
        engine.collectGarbage();
    }

    void MainComponent::paint (juce::Graphics& g)
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void MainComponent::resized()
    {
        auto area = getLocalBounds();

        topBar.setBounds (area.removeFromTop (40));
        statusBar.setBounds (area.removeFromBottom (30));

        const int half = area.getHeight() / 2;
        chainView.setBounds (area.removeFromTop (half));
        blockEditor.setBounds (area.removeFromTop (area.getHeight() / 2));
        reservedArea = area;

        // 볼륨 노브: INPUT 은 왼쪽 끝, OUTPUT 은 오른쪽 끝 (StatusBar 의 IN/OUT 과 같은 쪽)
        auto knobArea = reservedArea.reduced (8, 4);
        const int knobWidth = juce::jmin (100, knobArea.getWidth() / 2);
        inputKnob.setBounds (knobArea.removeFromLeft (knobWidth));
        outputKnob.setBounds (knobArea.removeFromRight (knobWidth));
    }
}
