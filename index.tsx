import React, {useState, useEffect} from "react"
import {createRoot} from "react-dom/client"
import Knob from "./components/Knob"
import SkewKnob from "./components/SkewKnob"
import BigSkewKnob from "./components/BigSkewKnob"
import PresetBar from "./components/PresetBar"
import DarkIcon from "./assets/dark.svg"
import LightIcon from "./assets/light.svg"
import parameters from "./processor/parameters.json"
import "./index.scss"

const darkColorList = {
    "--background": "#220E1A",
    "--textColor": "#FFFFFF"
}

const lightColorList = {
    "--background": "#FFBEE6",
    "--textColor": "#000000",
}

type ThemeContextType = {theme: string; setTheme: React.Dispatch<React.SetStateAction<string>>}
export const ThemeContext = React.createContext<ThemeContextType>({theme: "", setTheme: () => null})

const App: React.FunctionComponent = () => {
    const [theme, setTheme] = useState(localStorage.getItem("theme") || "light")
    
    useEffect(() => {
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [])

    useEffect(() => {
        const colorList = theme === "light" ? lightColorList : darkColorList
        for (const [key, color] of Object.entries(colorList)) {
            document.documentElement.style.setProperty(key, color)
        }
        localStorage.setItem("theme", theme)
        window.__JUCE__.backend.emitEvent("themeChange", {theme})
    }, [theme])

    const toggleTheme = () => {
        setTheme((prev) => prev === "light" ? "dark" : "light")
    }

    return (
        <div className="app">
            <ThemeContext.Provider value={{theme, setTheme}}>
            <div className="title-container">
                <span className="title-text">Cute Verb</span>
                {theme === "light" ? 
                <DarkIcon className="theme-icon" onClick={toggleTheme}/> :
                <LightIcon className="theme-icon" onClick={toggleTheme}/>}
            </div>
            <div className="knobs-container">
                <div className="column-container">
                    <Knob
                        label={parameters.mix.id.toUpperCase()}
                        parameterID={parameters.mix.id}
                        color="#FF4EA7"/>
                    <Knob
                        label={parameters.predelay.id.toUpperCase()}
                        parameterID={parameters.predelay.id}
                        display="ms"
                        color="#FF4EA7"/>
                </div>
                <div className="big-column-container">
                    <BigSkewKnob
                        label={parameters.decay.id.toUpperCase()}
                        parameterID={parameters.decay.id}
                        color="#FF4EA7"/>
                </div>
                <div className="column-container">
                    <Knob
                        label={parameters.size.id.toUpperCase()}
                        parameterID={parameters.size.id}
                        color="#FF4EA7"/>
                    <Knob
                        label={parameters.width.id.toUpperCase()}
                        parameterID={parameters.width.id}
                        color="#FF4EA7"/>
                </div>
                <div className="column-container">
                    <Knob
                        label={parameters.damping.id.toUpperCase()}
                        parameterID={parameters.damping.id}
                        color="#FF4EA7"/>
                    <Knob
                        label={parameters.diffusion.id.toUpperCase()}
                        parameterID={parameters.diffusion.id}
                        color="#FF4EA7"/>
                </div>
                <div className="column-container">
                    <SkewKnob
                        label={parameters.lowcut.id.toUpperCase()}
                        parameterID={parameters.lowcut.id}
                        color="#FF4EA7"/>
                    <SkewKnob
                        label={parameters.highcut.id.toUpperCase()}
                        parameterID={parameters.highcut.id}
                        color="#FF4EA7"/>
                </div>
            </div>
            <div className="preset-container">
                <PresetBar/>
            </div>
            </ThemeContext.Provider>
        </div>
    )

}

const root = createRoot(document.getElementById("root")!)
root.render(<App/>)