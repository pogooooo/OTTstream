import { BrowserRouter, Routes, Route } from 'react-router-dom';
import SignIn from "./components/SignIn.tsx";
import SignUp from "./components/SignUp.tsx";
import Main from "./components/Main.tsx";
import Header from "./components/Header.tsx";
import styled from "styled-components";
import VideoDetail from "./components/VideoDetail.tsx";

function App() {
    return (
        <Wrapper>
            <BrowserRouter>

                <Header />

                <Routes>
                    <Route path="/" element={<Main />} />
                    <Route path="/signIn" element={<SignIn />} />
                    <Route path="/signUp" element={<SignUp />} />
                    <Route path="/video/:id" element={<VideoDetail />} />
                </Routes>

            </BrowserRouter>
        </Wrapper>
    );
}

const Wrapper = styled.div`
    width: 100vw;
    height: 100vh;
`

export default App;
