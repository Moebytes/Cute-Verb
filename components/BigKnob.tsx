import React, {useId} from "react"
import {KnobHeadless, KnobHeadlessLabel, KnobHeadlessOutput, useKnobKeyboardControls} from "react-knob-headless"
import withJuceSlider, {WithJUCESliderProps} from "./withJuceSlider"
import functions from "../structures/Functions"
import "./styles/bigknob.scss"

interface Props {
    parameterID: string
    label: string
    color: string
    roundFunction?: (value: number) => number
    displayFunction?: (value: number) => string
    style?: React.CSSProperties
}

const BigKnob: React.FunctionComponent<Props & WithJUCESliderProps> = ({label, parameterID, 
    color, roundFunction, displayFunction, style, value, properties, onChange,
    reset, dragStart, dragEnd}) => {
    const knobID = useId()
    const labelID = useId()

    const minAngle = -145
    const maxAngle = 145
    const min = properties.start
    const max = properties.end
    const step = (max - min) / (properties.numSteps - 1)
    const angle = functions.remapRange(value, min, max, minAngle, maxAngle)

    if (!roundFunction) {
        roundFunction = (value: number) => value
    }

    if (!displayFunction) {
        displayFunction = (value: number) => `${value.toFixed(2)} s`
    }

    const keyboardHandler = useKnobKeyboardControls({
        valueRaw: value,
        valueMin: min,
        valueMax: max,
        step: step,
        stepLarger: step * 10,
        onValueRawChange: onChange,
    })

    return (
        <div className="big-knob-container" style={{...style}}>
            <KnobHeadlessLabel className="big-knob-label" id={labelID}>
                {label}
            </KnobHeadlessLabel>
            <KnobHeadless
                aria-label={parameterID}
                aria-labelledby={labelID}
                dragSensitivity={0.006}
                onValueRawChange={onChange}
                onDoubleClick={reset}
                valueMin={min}
                valueMax={max}
                valueRaw={value}
                valueRawDisplayFn={displayFunction}
                valueRawRoundFn={roundFunction}
                onDragStart={dragStart}
                onDragEnd={dragEnd}
                axis="y"
                style={{outline: "none"}}
                {...keyboardHandler}>
                <div className="big-knob" style={{backgroundColor: color}}>
                    <div className="big-knob-rotator" style={{transform: `rotate(${angle}deg)`}}>
                        <div className="big-knob-indicator"/>
                    </div>
                </div>
            </KnobHeadless>
            <KnobHeadlessOutput className="big-knob-value" htmlFor={knobID} style={{color}}>
                {displayFunction(value)}
            </KnobHeadlessOutput>
        </div>
    )
}

export default withJuceSlider(BigKnob)