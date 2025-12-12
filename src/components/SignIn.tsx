import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useDispatch } from 'react-redux';
import styled from 'styled-components';
import { signInApi } from '../utils/api.tsx';
import { login } from '../slices/UserSlice';

const SignIn = () => {
    const [nickname, setNickname] = useState('');
    const [password, setPassword] = useState('');
    const [message, setMessage] = useState('');

    const navigate = useNavigate();
    const dispatch = useDispatch();

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setMessage('로그인 시도 중...');

        const result = await signInApi({ nickname, password });

        if (result.success && result.token) {
            dispatch(login({
                nickname: nickname,
                token: result.token
            }));
            setMessage('로그인 성공! 메인 페이지로 이동합니다.');
            setTimeout(() => navigate('/'), 1000); // 메인 페이지로 리디렉션
        } else {
            setMessage(result.message);
        }
    };

    return (
        <AuthWrapper>
            <Form onSubmit={handleSubmit}>
                <h2>로그인</h2>
                <Input
                    type="text"
                    placeholder="닉네임"
                    value={nickname}
                    onChange={(e) => setNickname(e.target.value)}
                    required
                />
                <Input
                    type="password"
                    placeholder="비밀번호"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    required
                />
                <Button type="submit">로그인</Button>
                <p>계정이 없으신가요? <Link onClick={() => navigate('/signUp')}>회원가입</Link></p>
                <p style={{ color: message.includes('성공') ? 'green' : 'red' }}>{message}</p>
            </Form>
        </AuthWrapper>
    );
};

const AuthWrapper = styled.div`
    display: flex;
    justify-content: center;
    align-items: center;
    background-color: #f0f0f0; 
`;

const Form = styled.form`
    background: white;
    padding: 30px;
    border-radius: 10px;
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    display: flex;
    flex-direction: column;
    gap: 15px;
    width: 300px;
`;

const Input = styled.input`
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 5px;
`;

const Button = styled.button`
    padding: 10px;
    background-color: #007bff;
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    &:hover {
        background-color: #0056b3;
    }
`;

const Link = styled.span`
    color: #007bff;
    cursor: pointer;
    text-decoration: underline;
`;

export default SignIn;
